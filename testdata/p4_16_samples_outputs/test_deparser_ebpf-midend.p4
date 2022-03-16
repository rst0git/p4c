#include <core.p4>

@ethernetaddress typedef bit<48> EthernetAddress;
@ipv4address typedef bit<32> IPv4Address;
header Ethernet_h {
    EthernetAddress dstAddr;
    EthernetAddress srcAddr;
    bit<16>         etherType;
}

header IPv4_h {
    bit<4>      version;
    bit<4>      ihl;
    bit<8>      diffserv;
    bit<16>     totalLen;
    bit<16>     identification;
    bit<3>      flags;
    bit<13>     fragOffset;
    bit<8>      ttl;
    bit<8>      protocol;
    bit<16>     hdrChecksum;
    IPv4Address srcAddr;
    IPv4Address dstAddr;
}

parser parse<H>(packet_in packet, out H headers);
control pipeline<H>(inout H headers, out bool accept);
@deparser control deparser<H>(packet_out b, in H headers);
package ebpfFilter<H>(parse<H> prs, pipeline<H> filt, deparser<H> dprs);
struct Headers_t {
    Ethernet_h ethernet;
    IPv4_h     ipv4;
}

parser prs(packet_in p, out Headers_t headers) {
    state start {
        p.extract<Ethernet_h>(headers.ethernet);
        transition select(headers.ethernet.etherType) {
            16w0x800: ip;
            default: reject;
        }
    }
    state ip {
        p.extract<IPv4_h>(headers.ipv4);
        transition accept;
    }
}

control pipe(inout Headers_t headers, out bool pass) {
    @hidden action test_deparser_ebpf41() {
        headers.ipv4.setInvalid();
    }
    @hidden action test_deparser_ebpf39() {
        pass = true;
    }
    @hidden table tbl_test_deparser_ebpf39 {
        actions = {
            test_deparser_ebpf39();
        }
        const default_action = test_deparser_ebpf39();
    }
    @hidden table tbl_test_deparser_ebpf41 {
        actions = {
            test_deparser_ebpf41();
        }
        const default_action = test_deparser_ebpf41();
    }
    apply {
        tbl_test_deparser_ebpf39.apply();
        if (headers.ipv4.isValid()) {
            tbl_test_deparser_ebpf41.apply();
        }
    }
}

control dprs(packet_out packet, in Headers_t headers) {
    @hidden action test_deparser_ebpf48() {
        packet.emit<Ethernet_h>(headers.ethernet);
        packet.emit<IPv4_h>(headers.ipv4);
    }
    @hidden table tbl_test_deparser_ebpf48 {
        actions = {
            test_deparser_ebpf48();
        }
        const default_action = test_deparser_ebpf48();
    }
    apply {
        tbl_test_deparser_ebpf48.apply();
    }
}

ebpfFilter<Headers_t>(prs(), pipe(), dprs()) main;

