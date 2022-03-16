#include <core.p4>

#include "ebpf_headers.p4"

parser parse<H>(packet_in packet, out H headers);
control pipeline<H>(inout H headers, out bool accept);
@deparser
control deparser<H>(packet_out b, in H headers);

package ebpfFilter<H>(
    parse<H> prs,
    pipeline<H> filt,
    deparser<H> dprs
);

struct Headers_t {
    Ethernet_h ethernet;
    IPv4_h ipv4;
}

parser prs(packet_in p, out Headers_t headers)
{
    state start {
        p.extract(headers.ethernet);
        transition select(headers.ethernet.etherType) {
            16w0x800 : ip;
            default: reject;
        }
    }

    state ip {
        p.extract(headers.ipv4);
        transition accept;
    }
}

control pipe(inout Headers_t headers, out bool pass) {
    apply {
        pass = true;
        if (headers.ipv4.isValid()) {
            headers.ipv4.setInvalid();
        }
    }
}

control dprs(packet_out packet, in Headers_t headers) {
    apply {
        packet.emit(headers.ethernet);
        packet.emit(headers.ipv4);
    }
}

ebpfFilter(prs(), pipe(), dprs()) main;
