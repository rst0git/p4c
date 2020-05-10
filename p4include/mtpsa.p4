#ifndef __MTPSA_P4__
#define __MTPSA_P4__

#include <core.p4>

#ifndef _MULTI_TENANT_PORTABLE_SWITCH_ARCHITECTURE_P4_
#define _MULTI_TENANT_PORTABLE_SWITCH_ARCHITECTURE_P4_

#define MTPSA_ON_BMV2_CORE_TYPES
#ifdef MTPSA_ON_BMV2_CORE_TYPES

typedef bit<32> PortIdUint_t;
typedef bit<32> MulticastGroupUint_t;
typedef bit<16> CloneSessionIdUint_t;
typedef bit<8>  ClassOfServiceUint_t;
typedef bit<16> PacketLengthUint_t;
typedef bit<16> EgressInstanceUint_t;
typedef bit<64> TimestampUint_t;

@p4runtime_translation("p4.org/mtpsa/v1/PortId_t", 32)
type PortIdUint_t         PortId_t;
@p4runtime_translation("p4.org/mtpsa/v1/MulticastGroup_t", 32)
type MulticastGroupUint_t MulticastGroup_t;
@p4runtime_translation("p4.org/mtpsa/v1/CloneSessionId_t", 16)
type CloneSessionIdUint_t CloneSessionId_t;
@p4runtime_translation("p4.org/mtpsa/v1/ClassOfService_t", 8)
type ClassOfServiceUint_t ClassOfService_t;
@p4runtime_translation("p4.org/mtpsa/v1/PacketLength_t", 16)
type PacketLengthUint_t   PacketLength_t;
@p4runtime_translation("p4.org/mtpsa/v1/EgressInstance_t", 16)
type EgressInstanceUint_t EgressInstance_t;
@p4runtime_translation("p4.org/mtpsa/v1/Timestamp_t", 64)
type TimestampUint_t      Timestamp_t;
typedef error   ParserError_t;

const PortId_t MTPSA_PORT_RECIRCULATE = (PortId_t) 0xfffffffa;
const PortId_t MTPSA_PORT_CPU = (PortId_t) 0xfffffffd;

const CloneSessionId_t MTPSA_CLONE_SESSION_TO_CPU = (CloneSessionId_t) 0;

#endif  // MTPSA_ON_BMV2_CORE_TYPES

#ifndef MTPSA_ON_BMV2_CORE_TYPES
#error "Please define the following types for PSA and the PSA_EXAMPLE_CORE_TYPES macro"

typedef bit<unspecified> PortIdUint_t;
typedef bit<unspecified> MulticastGroupUint_t;
typedef bit<unspecified> CloneSessionIdUint_t;
typedef bit<unspecified> ClassOfServiceUint_t;
typedef bit<unspecified> PacketLengthUint_t;
typedef bit<unspecified> EgressInstanceUint_t;
typedef bit<unspecified> TimestampUint_t;

@p4runtime_translation("p4.org/mtpsa/v1/PortId_t", 32)
type PortIdUint_t         PortId_t;
@p4runtime_translation("p4.org/mtpsa/v1/MulticastGroup_t", 32)
type MulticastGroupUint_t MulticastGroup_t;
@p4runtime_translation("p4.org/mtpsa/v1/CloneSessionId_t", 16)
type CloneSessionIdUint_t CloneSessionId_t;
@p4runtime_translation("p4.org/mtpsa/v1/ClassOfService_t", 8)
type ClassOfServiceUint_t ClassOfService_t;
@p4runtime_translation("p4.org/mtpsa/v1/PacketLength_t", 16)
type PacketLengthUint_t   PacketLength_t;
@p4runtime_translation("p4.org/mtpsa/v1/EgressInstance_t", 16)
type EgressInstanceUint_t EgressInstance_t;
@p4runtime_translation("p4.org/mtpsa/v1/Timestamp_t", 64)
type TimestampUint_t      Timestamp_t;
typedef error   ParserError_t;

const PortId_t MTPSA_PORT_RECIRCULATE = (PortId_t) unspecified;
const PortId_t MTPSA_PORT_CPU = (PortId_t) unspecified;

const CloneSessionId_t MTPSA_CLONE_SESSION_TO_CPU = (CloneSessiontId_t) unspecified;

#endif  // #ifndef PSA_EXAMPLE_CORE_TYPES

typedef bit<32> PortIdInHeaderUint_t;
typedef bit<32> MulticastGroupInHeaderUint_t;
typedef bit<16> CloneSessionIdInHeaderUint_t;
typedef bit<8>  ClassOfServiceInHeaderUint_t;
typedef bit<16> PacketLengthInHeaderUint_t;
typedef bit<16> EgressInstanceInHeaderUint_t;
typedef bit<64> TimestampInHeaderUint_t;

@p4runtime_translation("p4.org/mtpsa/v1/PortIdInHeader_t", 32)
type  PortIdInHeaderUint_t         PortIdInHeader_t;
@p4runtime_translation("p4.org/mtpsa/v1/MulticastGroupInHeader_t", 32)
type  MulticastGroupInHeaderUint_t MulticastGroupInHeader_t;
@p4runtime_translation("p4.org/mtpsa/v1/CloneSessionIdInHeader_t", 16)
type  CloneSessionIdInHeaderUint_t CloneSessionIdInHeader_t;
@p4runtime_translation("p4.org/mtpsa/v1/ClassOfServiceInHeader_t", 8)
type  ClassOfServiceInHeaderUint_t ClassOfServiceInHeader_t;
@p4runtime_translation("p4.org/mtpsa/v1/PacketLengthInHeader_t", 16)
type  PacketLengthInHeaderUint_t   PacketLengthInHeader_t;
@p4runtime_translation("p4.org/mtpsa/v1/EgressInstanceInHeader_t", 16)
type  EgressInstanceInHeaderUint_t EgressInstanceInHeader_t;
@p4runtime_translation("p4.org/mtpsa/v1/TimestampInHeader_t", 64)
type  TimestampInHeaderUint_t      TimestampInHeader_t;

PortId_t mtpsa_PortId_header_to_int (in PortIdInHeader_t x) {
    return (PortId_t) (PortIdUint_t) (PortIdInHeaderUint_t) x;
}
MulticastGroup_t mtpsa_MulticastGroup_header_to_int (in MulticastGroupInHeader_t x) {
    return (MulticastGroup_t) (MulticastGroupUint_t) (MulticastGroupInHeaderUint_t) x;
}
CloneSessionId_t mtpsa_CloneSessionId_header_to_int (in CloneSessionIdInHeader_t x) {
    return (CloneSessionId_t) (CloneSessionIdUint_t) (CloneSessionIdInHeaderUint_t) x;
}
ClassOfService_t mtpsa_ClassOfService_header_to_int (in ClassOfServiceInHeader_t x) {
    return (ClassOfService_t) (ClassOfServiceUint_t) (ClassOfServiceInHeaderUint_t) x;
}
PacketLength_t mtpsa_PacketLength_header_to_int (in PacketLengthInHeader_t x) {
    return (PacketLength_t) (PacketLengthUint_t) (PacketLengthInHeaderUint_t) x;
}
EgressInstance_t mtpsa_EgressInstance_header_to_int (in EgressInstanceInHeader_t x) {
    return (EgressInstance_t) (EgressInstanceUint_t) (EgressInstanceInHeaderUint_t) x;
}
Timestamp_t mtpsa_Timestamp_header_to_int (in TimestampInHeader_t x) {
    return (Timestamp_t) (TimestampUint_t) (TimestampInHeaderUint_t) x;
}

PortIdInHeader_t mtpsa_PortId_int_to_header (in PortId_t x) {
    return (PortIdInHeader_t) (PortIdInHeaderUint_t) (PortIdUint_t) x;
}
MulticastGroupInHeader_t mtpsa_MulticastGroup_int_to_header (in MulticastGroup_t x) {
    return (MulticastGroupInHeader_t) (MulticastGroupInHeaderUint_t) (MulticastGroupUint_t) x;
}
CloneSessionIdInHeader_t mtpsa_CloneSessionId_int_to_header (in CloneSessionId_t x) {
    return (CloneSessionIdInHeader_t) (CloneSessionIdInHeaderUint_t) (CloneSessionIdUint_t) x;
}
ClassOfServiceInHeader_t mtpsa_ClassOfService_int_to_header (in ClassOfService_t x) {
    return (ClassOfServiceInHeader_t) (ClassOfServiceInHeaderUint_t) (ClassOfServiceUint_t) x;
}
PacketLengthInHeader_t mtpsa_PacketLength_int_to_header (in PacketLength_t x) {
    return (PacketLengthInHeader_t) (PacketLengthInHeaderUint_t) (PacketLengthUint_t) x;
}
EgressInstanceInHeader_t mtpsa_EgressInstance_int_to_header (in EgressInstance_t x) {
    return (EgressInstanceInHeader_t) (EgressInstanceInHeaderUint_t) (EgressInstanceUint_t) x;
}
TimestampInHeader_t mtpsa_Timestamp_int_to_header (in Timestamp_t x) {
    return (TimestampInHeader_t) (TimestampInHeaderUint_t) (TimestampUint_t) x;
}

enum MTPSA_IdleTimeout_t {
  NO_TIMEOUT,
  NOTIFY_CONTROL
};

enum MTPSA_PacketPath_t {
    NORMAL,     /// Packet received by ingress that is none of the cases below.
    NORMAL_UNICAST,   /// Normal packet received by egress which is unicast
    NORMAL_MULTICAST, /// Normal packet received by egress which is multicast
    CLONE_I2E,  /// Packet created via a clone operation in ingress, destined for egress
    CLONE_E2E,  /// Packet created via a clone operation in egress, destined for egress
    RESUBMIT,   /// Packet arrival is the result of a resubmit operation
    RECIRCULATE /// Packet arrival is the result of a recirculate operation
}

struct mtpsa_ingress_parser_input_metadata_t {
  PortId_t                 ingress_port;
  MTPSA_PacketPath_t       packet_path;
}

struct mtpsa_egress_parser_input_metadata_t {
  PortId_t                 egress_port;
  MTPSA_PacketPath_t       packet_path;
}

struct mtpsa_ingress_input_metadata_t {
  PortId_t                 ingress_port;
  MTPSA_PacketPath_t       packet_path;
  Timestamp_t              ingress_timestamp;
  ParserError_t            parser_error;
}

struct mtpsa_ingress_output_metadata_t {
  ClassOfService_t         class_of_service; // 0
  bool                     clone;            // false
  CloneSessionId_t         clone_session_id; // initial value is undefined
  bool                     drop;             // true
  bool                     resubmit;         // false
  MulticastGroup_t         multicast_group;  // 0
  PortId_t                 egress_port;      // initial value is undefined
  bit<16>                  user_id;
}

struct mtpsa_egress_input_metadata_t {
  ClassOfService_t         class_of_service;
  PortId_t                 egress_port;
  MTPSA_PacketPath_t       packet_path;
  EgressInstance_t         instance;       /// instance comes from the PacketReplicationEngine
  Timestamp_t              egress_timestamp;
  ParserError_t            parser_error;
}

struct mtpsa_egress_deparser_input_metadata_t {
  PortId_t                 egress_port;
}

struct mtpsa_egress_output_metadata_t {
  bool                     clone;         // false
  CloneSessionId_t         clone_session_id; // initial value is undefined
  bool                     drop;          // false
}

extern bool mtpsa_clone_i2e(in mtpsa_ingress_output_metadata_t istd);
extern bool mtpsa_resubmit(in mtpsa_ingress_output_metadata_t istd);
extern bool mtpsa_normal(in mtpsa_ingress_output_metadata_t istd);
extern bool mtpsa_clone_e2e(in mtpsa_egress_output_metadata_t istd);
extern bool mtpsa_recirculate(in mtpsa_egress_output_metadata_t istd,
                            in mtpsa_egress_deparser_input_metadata_t edstd);

extern void assert(in bool check);
extern void assume(in bool check);

match_kind {
    range,   /// Used to represent min..max intervals
    selector /// Used for dynamic action selection via the ActionSelector extern
}

action send_to_port(inout mtpsa_ingress_output_metadata_t meta,
                    in PortId_t egress_port)
{
    meta.drop = false;
    meta.multicast_group = (MulticastGroup_t) 0;
    meta.egress_port = egress_port;
}

action multicast(inout mtpsa_ingress_output_metadata_t meta,
                 in MulticastGroup_t multicast_group)
{
    meta.drop = false;
    meta.multicast_group = multicast_group;
}

action ingress_drop(inout mtpsa_ingress_output_metadata_t meta)
{
    meta.drop = true;
}

action egress_drop(inout mtpsa_egress_output_metadata_t meta)
{
    meta.drop = true;
}

extern PacketReplicationEngine {
    PacketReplicationEngine();
}

extern BufferingQueueingEngine {
    BufferingQueueingEngine();
}

enum MTPSA_HashAlgorithm_t {
  IDENTITY,
  CRC32,
  CRC32_CUSTOM,
  CRC16,
  CRC16_CUSTOM,
  ONES_COMPLEMENT16,  /// One's complement 16-bit sum used for IPv4 headers, TCP, and UDP.
  TARGET_DEFAULT      /// target implementation defined
}

extern Hash<O> {
  Hash(MTPSA_HashAlgorithm_t algo);
  O get_hash<D>(in D data);
  O get_hash<T, D>(in T base, in D data, in T max);
}

extern Checksum<W> {
  Checksum(MTPSA_HashAlgorithm_t hash);
  void clear();
  void update<T>(in T data);
  W    get();
}

extern InternetChecksum {
  InternetChecksum();
  void clear();
  void add<T>(in T data);
  void subtract<T>(in T data);
  bit<16> get();
  bit<16> get_state();
  void set_state(in bit<16> checksum_state);
}

enum MTPSA_CounterType_t {
    PACKETS,
    BYTES,
    PACKETS_AND_BYTES
}

extern Counter<W, S> {
  Counter(bit<32> n_counters, MTPSA_CounterType_t type);
  void count(in S index);
}

extern DirectCounter<W> {
  DirectCounter(MTPSA_CounterType_t type);
  void count();
}

enum MTPSA_MeterType_t {
    PACKETS,
    BYTES
}

enum MTPSA_MeterColor_t { RED, GREEN, YELLOW }

extern Meter<S> {
  Meter(bit<32> n_meters, MTPSA_MeterType_t type);
  MTPSA_MeterColor_t execute(in S index, in MTPSA_MeterColor_t color);
  MTPSA_MeterColor_t execute(in S index);
}

extern DirectMeter {
  DirectMeter(MTPSA_MeterType_t type);
  MTPSA_MeterColor_t execute(in MTPSA_MeterColor_t color);
  MTPSA_MeterColor_t execute();
}

extern Register<T, S> {
  Register(bit<32> size);
  Register(bit<32> size, T initial_value);

  T    read  (in S index);
  void write (in S index, in T value);
}

extern Random<T> {
  Random(T min, T max);
  T read();
}

extern ActionProfile {
  ActionProfile(bit<32> size);
}

extern ActionSelector {
  ActionSelector(MTPSA_HashAlgorithm_t algo, bit<32> size, bit<32> outputWidth);
}

extern Digest<T> {
  Digest();                       /// define a digest stream to the control plane
  void pack(in T data);           /// emit data into the stream
}

parser IngressParser<H, M, RESUBM, RECIRCM>(
    packet_in buffer,
    out H parsed_hdr,
    inout M user_meta,
    in mtpsa_ingress_parser_input_metadata_t istd,
    in RESUBM resubmit_meta,
    in RECIRCM recirculate_meta);

control Ingress<H, M>(
    inout H hdr, inout M user_meta,
    in    mtpsa_ingress_input_metadata_t  istd,
    inout mtpsa_ingress_output_metadata_t ostd);

control IngressDeparser<H, M, CI2EM, RESUBM, NM>(
    packet_out buffer,
    out CI2EM clone_i2e_meta,
    out RESUBM resubmit_meta,
    out NM normal_meta,
    inout H hdr,
    in M meta,
    in mtpsa_ingress_output_metadata_t istd);

parser EgressParser<H, M, NM, CI2EM, CE2EM>(
    packet_in buffer,
    out H parsed_hdr,
    inout M user_meta,
    in mtpsa_egress_parser_input_metadata_t istd,
    in NM normal_meta,
    in CI2EM clone_i2e_meta,
    in CE2EM clone_e2e_meta);

control Egress<H, M>(
    inout H hdr, inout M user_meta,
    in    mtpsa_egress_input_metadata_t  istd,
    inout mtpsa_egress_output_metadata_t ostd);

control EgressDeparser<H, M, CE2EM, RECIRCM>(
    packet_out buffer,
    out CE2EM clone_e2e_meta,
    out RECIRCM recirculate_meta,
    inout H hdr,
    in M meta,
    in mtpsa_egress_output_metadata_t istd,
    in mtpsa_egress_deparser_input_metadata_t edstd);

package IngressPipeline<IH, IM, NM, CI2EM, RESUBM, RECIRCM>(
    IngressParser<IH, IM, RESUBM, RECIRCM> ip,
    Ingress<IH, IM> ig,
    IngressDeparser<IH, IM, CI2EM, RESUBM, NM> id);

package MTPSA_Switch<IH, IM, EH, EM, NM, CI2EM, CE2EM, RESUBM, RECIRCM> (
    IngressPipeline<IH, IM, NM, CI2EM, RESUBM, RECIRCM> ingress,
    PacketReplicationEngine pre,
    EgressParser<EH, EM, NM, CI2EM, CE2EM> ep,
    Egress<EH, EM> eg,
    EgressDeparser<EH, EM, CE2EM, RECIRCM> ed,
    BufferingQueueingEngine bqe);

#endif  /* _MULTI_TENANT_PORTABLE_SWITCH_ARCHITECTURE_P4_ */

#endif   // __MTPSA_P4__
