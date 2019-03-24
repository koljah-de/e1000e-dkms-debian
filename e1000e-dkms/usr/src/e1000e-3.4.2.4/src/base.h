/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 1999 - 2019 Intel Corporation. */

#ifndef _E1000E_BASE_H_
#define _E1000E_BASE_H_

/* forward declaration */
s32 e1000_init_nvm_params_base(struct e1000_hw *hw);
s32 e1000_init_hw_82575(struct e1000_hw *hw);
extern void e1000_rx_fifo_flush_base(struct e1000_hw *hw);

/* Transmit Descriptor - Advanced */
union e1000_adv_tx_desc {
	struct {
		__le64 buffer_addr;	/* Address of descriptor's data buf */
		__le32 cmd_type_len;
		__le32 olinfo_status;
	} read;
	struct {
		__le64 rsvd;	/* Reserved */
		__le32 nxtseq_seed;
		__le32 status;
	} wb;
};

/* Adv Transmit Descriptor Config Masks */
#define E1000_ADVTXD_DTYP_CTXT	0x00200000	/* Advanced Context Descriptor */
#define E1000_ADVTXD_DTYP_DATA	0x00300000	/* Advanced Data Descriptor */
#define E1000_ADVTXD_DCMD_EOP	0x01000000	/* End of Packet */
#define E1000_ADVTXD_DCMD_IFCS	0x02000000	/* Insert FCS (Ethernet CRC) */
#define E1000_ADVTXD_DCMD_RS	0x08000000	/* Report Status */
#define E1000_ADVTXD_DCMD_DDTYP_ISCSI	0x10000000	/* DDP hdr type or iSCSI */
#define E1000_ADVTXD_DCMD_DEXT	0x20000000	/* Descriptor extension (1=Adv) */
#define E1000_ADVTXD_DCMD_VLE	0x40000000	/* VLAN pkt enable */
#define E1000_ADVTXD_DCMD_TSE	0x80000000	/* TCP Seg enable */
#define E1000_ADVTXD_MAC_LINKSEC	0x00040000	/* Apply LinkSec on pkt */
#define E1000_ADVTXD_MAC_TSTAMP		0x00080000	/* IEEE1588 Timestamp pkt */
#define E1000_ADVTXD_STAT_SN_CRC	0x00000002	/* NXTSEQ/SEED prsnt in WB */
#define E1000_ADVTXD_IDX_SHIFT		4	/* Adv desc Index shift */
#define E1000_ADVTXD_POPTS_ISCO_1ST	0x00000000	/* 1st TSO of iSCSI PDU */
#define E1000_ADVTXD_POPTS_ISCO_MDL	0x00000800	/* Middle TSO of iSCSI PDU */
#define E1000_ADVTXD_POPTS_ISCO_LAST	0x00001000	/* Last TSO of iSCSI PDU */
/* 1st & Last TSO-full iSCSI PDU*/
#define E1000_ADVTXD_POPTS_ISCO_FULL	0x00001800
#define E1000_ADVTXD_POPTS_IPSEC	0x00000400	/* IPSec offload request */
#define E1000_ADVTXD_PAYLEN_SHIFT	14	/* Adv desc PAYLEN shift */

#define E1000_RAR_ENTRIES_82575	16

/* Receive Descriptor - Advanced */
union e1000_adv_rx_desc {
	struct {
		__le64 pkt_addr;	/* Packet buffer address */
		__le64 hdr_addr;	/* Header buffer address */
	} read;
	struct {
		struct {
			union {
				__le32 data;
				struct {
					__le16 pkt_info;	/*RSS type, Pkt type */
					/* Split Header, header buffer len */
					__le16 hdr_info;
				} hs_rss;
			} lo_dword;
			union {
				__le32 rss;	/* RSS Hash */
				struct {
					__le16 ip_id;	/* IP id */
					__le16 csum;	/* Packet Checksum */
				} csum_ip;
			} hi_dword;
		} lower;
		struct {
			__le32 status_error;	/* ext status/error */
			__le16 length;	/* Packet length */
			__le16 vlan;	/* VLAN tag */
		} upper;
	} wb;			/* writeback */
};

/* Additional Transmit Descriptor Control definitions */
#define E1000_TXDCTL_QUEUE_ENABLE	0x02000000	/* Ena specific Tx Queue */

/* Additional Receive Descriptor Control definitions */
#define E1000_RXDCTL_QUEUE_ENABLE	0x02000000	/* Ena specific Rx Queue */

/* SRRCTL bit definitions */
#define E1000_SRRCTL_BSIZEPKT_SHIFT		10	/* Shift _right_ */
#define E1000_SRRCTL_BSIZEHDRSIZE_SHIFT		2	/* Shift _left_ */
#define E1000_SRRCTL_DESCTYPE_ADV_ONEBUF	0x02000000

#endif /* _E1000E_BASE_H_ */
