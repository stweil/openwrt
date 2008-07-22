/*
 * Copyright (c) 2008 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef CORE_H
#define CORE_H

#include <linux/version.h>
#include <linux/autoconf.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <asm/byteorder.h>
#include <linux/scatterlist.h>
#include <asm/page.h>
#include <net/mac80211.h>

#include "ath9k.h"
#include "rc.h"

struct ath_node;

/******************/
/* Utility macros */
/******************/

/* An attempt will be made to merge these link list helpers upstream
 * instead */

static inline void __list_splice_tail(const struct list_head *list,
				 struct list_head *head)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;
	struct list_head *current_tail = head->prev;

	current_tail->next = first;
	last->next = head;
	head->prev = last;
	first->prev = current_tail;
}

static inline void __list_cut_position(struct list_head *list,
		struct list_head *head, struct list_head *entry)
{
	struct list_head *new_first =
		(entry->next != head) ? entry->next : head;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

/**
 * list_splice_tail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void list_splice_tail(const struct list_head *list,
				struct list_head *head)
{
	if (!list_empty(list))
		__list_splice_tail(list, head);
}

/**
 * list_splice_tail_init - join two lists, each list being a queue, and
 * 	reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void list_splice_tail_init(struct list_head *list,
				    struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice_tail(list, head);
		INIT_LIST_HEAD(list);
	}
}

/**
 * list_cut_position - cut a list into two
 * @list: a new list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *	and if so we won't won't cut the list
 */
static inline void list_cut_position(struct list_head *list,
		struct list_head *head, struct list_head *entry)
{
	BUG_ON(list_empty(head));
	if (list_is_singular(head))
		BUG_ON(head->next != entry && head != entry);
	if (entry == head)
		INIT_LIST_HEAD(list);
	else
		__list_cut_position(list, head, entry);
}

/* Macro to expand scalars to 64-bit objects */

#define	ito64(x) (sizeof(x) == 8) ? \
	(((unsigned long long int)(x)) & (0xff)) : \
	(sizeof(x) == 16) ? \
	(((unsigned long long int)(x)) & 0xffff) : \
	((sizeof(x) == 32) ?			\
	 (((unsigned long long int)(x)) & 0xffffffff) : \
	(unsigned long long int)(x))

/* increment with wrap-around */
#define INCR(_l, _sz)   do { \
	(_l)++; \
	(_l) &= ((_sz) - 1); \
	} while (0)

/* decrement with wrap-around */
#define DECR(_l,  _sz)  do { \
	(_l)--; \
	(_l) &= ((_sz) - 1); \
	} while (0)

#define A_MAX(a, b) ((a) > (b) ? (a) : (b))

#define ASSERT(exp) do {			\
		if (unlikely(!(exp))) {		\
			BUG();			\
		}				\
	} while (0)

#define KASSERT(exp, msg) do {			\
		if (unlikely(!(exp))) {         \
			printk msg;		\
			BUG();			\
		}				\
	} while (0)

/* XXX: remove */
#define memzero(_buf, _len) memset(_buf, 0, _len)

#define get_dma_mem_context(var, field) (&((var)->field))
#define copy_dma_mem_context(dst, src)  (*dst = *src)

#define ATH9K_BH_STATUS_INTACT		0
#define ATH9K_BH_STATUS_CHANGE		1

#define	ATH_TXQ_SETUP(sc, i)        ((sc)->sc_txqsetup & (1<<i))

static inline unsigned long get_timestamp(void)
{
	return ((jiffies / HZ) * 1000) + (jiffies % HZ) * (1000 / HZ);
}

/*************/
/* Debugging */
/*************/

enum ATH_DEBUG {
	ATH_DEBUG_XMIT          = 0x00000001,   /* basic xmit operation */
	ATH_DEBUG_RECV          = 0x00000002,   /* basic recv operation */
	ATH_DEBUG_BEACON        = 0x00000004,   /* beacon handling */
	ATH_DEBUG_TX_PROC       = 0x00000008,   /* tx ISR proc */
	ATH_DEBUG_RX_PROC       = 0x00000010,   /* rx ISR proc */
	ATH_DEBUG_BEACON_PROC   = 0x00000020,   /* beacon ISR proc */
	ATH_DEBUG_RATE          = 0x00000040,   /* rate control */
	ATH_DEBUG_CONFIG        = 0x00000080,   /* configuration */
	ATH_DEBUG_KEYCACHE      = 0x00000100,   /* key cache management */
	ATH_DEBUG_NODE          = 0x00000200,   /* node management */
	ATH_DEBUG_AGGR		= 0x00000400,	/* Aggregation */
	ATH_DEBUG_CWM		= 0x00000800,	/* Channel Width Management */
	ATH_DEBUG_FATAL         = 0x00001000,   /* fatal errors */
	ATH_DEBUG_ANY           = 0xffffffff
};

#define DBG_DEFAULT (ATH_DEBUG_FATAL)

#define	DPRINTF(sc, _m, _fmt, ...) do {			\
		if (sc->sc_debug & (_m))                \
			printk(_fmt , ##__VA_ARGS__);	\
	} while (0)

/***************************/
/* Load-time Configuration */
/***************************/

/* Per-instance load-time (note: NOT run-time) configurations
 * for Atheros Device */
struct ath_config {
	u_int8_t    chainmask_sel; /* enable automatic tx chainmask selection */
	u_int32_t   ath_aggr_prot;
	u_int16_t   txpowlimit;
	u_int16_t   txpowlimit_override;
	u_int8_t    cabqReadytime; /* Cabq Readytime % */
	u_int8_t    swBeaconProcess; /* Process received beacons
					in SW (vs HW) */
};

/***********************/
/* Chainmask Selection */
/***********************/

#define ATH_CHAINMASK_SEL_TIMEOUT	   6000
/* Default - Number of last RSSI values that is used for
 * chainmask selection */
#define ATH_CHAINMASK_SEL_RSSI_CNT	   10
/* Means use 3x3 chainmask instead of configured chainmask */
#define ATH_CHAINMASK_SEL_3X3		   7
/* Default - Rssi threshold below which we have to switch to 3x3 */
#define ATH_CHAINMASK_SEL_UP_RSSI_THRES	   20
/* Default - Rssi threshold above which we have to switch to
 * user configured values */
#define ATH_CHAINMASK_SEL_DOWN_RSSI_THRES  35
/* Struct to store the chainmask select related info */
struct ath_chainmask_sel {
	struct timer_list   timer;
	int                 cur_tx_mask; 	/* user configured or 3x3 */
	int                 cur_rx_mask; 	/* user configured or 3x3 */
	int                 tx_avgrssi;
	u8                  switch_allowed:1, 	/* timer will set this */
			    cm_sel_enabled:1;
};

int ath_chainmask_sel_logic(struct ath_softc *sc, struct ath_node *an);

/*************************/
/* Descriptor Management */
/*************************/

/* Number of descriptors per buffer. The only case where we see skbuff
chains is due to FF aggregation in the driver. */
#define	ATH_TXDESC	    1
/* if there's more fragment for this MSDU */
#define ATH_BF_MORE_MPDU    1
#define ATH_TXBUF_RESET(_bf) do {				\
		(_bf)->bf_status = 0;				\
		(_bf)->bf_lastbf = NULL;			\
		(_bf)->bf_lastfrm = NULL;			\
		(_bf)->bf_next = NULL;				\
		memzero(&((_bf)->bf_state),			\
			    sizeof(struct ath_buf_state));	\
	} while (0)

struct ath_buf_state {
	int bfs_nframes;	/* # frames in aggregate */
	u_int16_t bfs_al;	/* length of aggregate */
	u_int16_t bfs_frmlen;	/* length of frame */
	int bfs_seqno;		/* sequence number */
	int bfs_tidno;		/* tid of this frame */
	int bfs_retries;	/* current retries */
	struct ath_rc_series bfs_rcs[4];	/* rate series */
	u8 bfs_isdata:1;	/* is a data frame/aggregate */
	u8 bfs_isaggr:1;	/* is an aggregate */
	u8 bfs_isampdu:1;	/* is an a-mpdu, aggregate or not */
	u8 bfs_ht:1;		/* is an HT frame */
	u8 bfs_isretried:1;	/* is retried */
	u8 bfs_isxretried:1;	/* is excessive retried */
	u8 bfs_shpreamble:1;	/* is short preamble */
	u8 bfs_isbar:1;	/* is a BAR */
	u8 bfs_ispspoll:1;	/* is a PS-Poll */
	u8 bfs_aggrburst:1;	/* is a aggr burst */
	u8 bfs_calcairtime:1;	/* requests airtime be calculated
				when set for tx frame */
	int bfs_rifsburst_elem;	/* RIFS burst/bar */
	int bfs_nrifsubframes;	/* # of elements in burst */
	enum hal_key_type bfs_keytype;	/* key type use to encrypt this frame */
};

#define bf_nframes      	bf_state.bfs_nframes
#define bf_al           	bf_state.bfs_al
#define bf_frmlen       	bf_state.bfs_frmlen
#define bf_retries      	bf_state.bfs_retries
#define bf_seqno        	bf_state.bfs_seqno
#define bf_tidno        	bf_state.bfs_tidno
#define bf_rcs          	bf_state.bfs_rcs
#define bf_isdata       	bf_state.bfs_isdata
#define bf_isaggr       	bf_state.bfs_isaggr
#define bf_isampdu      	bf_state.bfs_isampdu
#define bf_ht           	bf_state.bfs_ht
#define bf_isretried    	bf_state.bfs_isretried
#define bf_isxretried   	bf_state.bfs_isxretried
#define bf_shpreamble   	bf_state.bfs_shpreamble
#define bf_rifsburst_elem  	bf_state.bfs_rifsburst_elem
#define bf_nrifsubframes  	bf_state.bfs_nrifsubframes
#define bf_keytype      	bf_state.bfs_keytype
#define bf_isbar        	bf_state.bfs_isbar
#define bf_ispspoll     	bf_state.bfs_ispspoll
#define bf_aggrburst    	bf_state.bfs_aggrburst
#define bf_calcairtime  	bf_state.bfs_calcairtime

/*
 * Abstraction of a contiguous buffer to transmit/receive.  There is only
 * a single hw descriptor encapsulated here.
 */

struct ath_buf {
	struct list_head list;
	struct list_head *last;
	struct ath_buf *bf_lastbf;	/* last buf of this unit (a frame or
					an aggregate) */
	struct ath_buf *bf_lastfrm;	/* last buf of this frame */
	struct ath_buf *bf_next;	/* next subframe in the aggregate */
	struct ath_buf *bf_rifslast;	/* last buf for RIFS burst */
	void *bf_mpdu;			/* enclosing frame structure */
	void *bf_node;			/* pointer to the node */
	struct ath_desc *bf_desc;	/* virtual addr of desc */
	dma_addr_t bf_daddr;		/* physical addr of desc */
	dma_addr_t bf_buf_addr;		/* physical addr of data buffer */
	u_int32_t bf_status;
	u_int16_t bf_flags;		/* tx descriptor flags */
	struct ath_buf_state bf_state;	/* buffer state */
	dma_addr_t bf_dmacontext;
};

/*
 * reset the rx buffer.
 * any new fields added to the athbuf and require
 * reset need to be added to this macro.
 * currently bf_status is the only one requires that
 * requires reset.
 */
#define ATH_RXBUF_RESET(_bf)    ((_bf)->bf_status = 0)

/* hw processing complete, desc processed by hal */
#define ATH_BUFSTATUS_DONE      0x00000001
/* hw processing complete, desc hold for hw */
#define ATH_BUFSTATUS_STALE     0x00000002
/* Rx-only: OS is done with this packet and it's ok to queued it to hw */
#define ATH_BUFSTATUS_FREE      0x00000004

/* DMA state for tx/rx descriptors */

struct ath_descdma {
	const char *dd_name;
	struct ath_desc *dd_desc;	/* descriptors  */
	dma_addr_t dd_desc_paddr;	/* physical addr of dd_desc  */
	u_int32_t dd_desc_len;		/* size of dd_desc  */
	struct ath_buf *dd_bufptr;	/* associated buffers */
	dma_addr_t dd_dmacontext;
};

/* Abstraction of a received RX MPDU/MMPDU, or a RX fragment */

struct ath_rx_context {
	struct ath_buf *ctx_rxbuf;	/* associated ath_buf for rx */
};
#define ATH_RX_CONTEXT(skb) ((struct ath_rx_context *)skb->cb)

int ath_descdma_setup(struct ath_softc *sc,
		      struct ath_descdma *dd,
		      struct list_head *head,
		      const char *name,
		      int nbuf,
		      int ndesc);
int ath_desc_alloc(struct ath_softc *sc);
void ath_desc_free(struct ath_softc *sc);
void ath_descdma_cleanup(struct ath_softc *sc,
			 struct ath_descdma *dd,
			 struct list_head *head);

/******/
/* RX */
/******/

#define ATH_MAX_ANTENNA          3
#define ATH_RXBUF                512
#define ATH_RX_TIMEOUT           40      /* 40 milliseconds */
#define WME_NUM_TID              16
#define IEEE80211_BAR_CTL_TID_M  0xF000  /* tid mask */
#define IEEE80211_BAR_CTL_TID_S  2       /* tid shift */

enum ATH_RX_TYPE {
	ATH_RX_NON_CONSUMED = 0,
	ATH_RX_CONSUMED
};

/* per frame rx status block */
struct ath_recv_status {
	u_int64_t tsf;		/* mac tsf */
	int8_t rssi;		/* RSSI (noise floor ajusted) */
	int8_t rssictl[ATH_MAX_ANTENNA];	/* RSSI (noise floor ajusted) */
	int8_t rssiextn[ATH_MAX_ANTENNA];	/* RSSI (noise floor ajusted) */
	int8_t abs_rssi;	/* absolute RSSI */
	u_int8_t rateieee;	/* data rate received (IEEE rate code) */
	u_int8_t ratecode;	/* phy rate code */
	int rateKbps;		/* data rate received (Kbps) */
	int antenna;		/* rx antenna */
	int flags;		/* status of associated skb */
#define ATH_RX_FCS_ERROR        0x01
#define ATH_RX_MIC_ERROR        0x02
#define ATH_RX_DECRYPT_ERROR    0x04
#define ATH_RX_RSSI_VALID       0x08
/* if any of ctl,extn chainrssis are valid */
#define ATH_RX_CHAIN_RSSI_VALID 0x10
/* if extn chain rssis are valid */
#define ATH_RX_RSSI_EXTN_VALID  0x20
/* set if 40Mhz, clear if 20Mhz */
#define ATH_RX_40MHZ            0x40
/* set if short GI, clear if full GI */
#define ATH_RX_SHORT_GI         0x80
};

struct ath_rxbuf {
	struct sk_buff         		*rx_wbuf; /* buffer */
	unsigned long               	rx_time; /* system time when received */
	struct ath_recv_status   	rx_status; /* cached rx status */
};

/* Per-TID aggregate receiver state for a node */
struct ath_arx_tid {
	struct ath_node     *an;        /* parent ath node */
	struct ath_rxbuf    *rxbuf; 	/* re-ordering buffer */
	struct timer_list   timer;
	spinlock_t          tidlock;    /* lock to protect this TID structure */
	int                 baw_head;   /* seq_next at head */
	int                 baw_tail;   /* tail of block-ack window */
	int                 seq_reset;  /* need to reset start sequence */
	int         	    addba_exchangecomplete;
	u_int16_t           seq_next;   /* next expected sequence */
	u_int16_t           baw_size;   /* block-ack window size */
};

/* Per-node receiver aggregate state */
struct ath_arx {
	struct ath_arx_tid  tid[WME_NUM_TID];
};

void ath_setrxfilter(struct ath_softc *sc);
int ath_startrecv(struct ath_softc *sc);
enum hal_bool ath_stoprecv(struct ath_softc *sc);
void ath_flushrecv(struct ath_softc *sc);
u_int32_t ath_calcrxfilter(struct ath_softc *sc);
void ath_rx_node_init(struct ath_softc *sc, struct ath_node *an);
void ath_rx_node_free(struct ath_softc *sc, struct ath_node *an);
void ath_rx_node_cleanup(struct ath_softc *sc, struct ath_node *an);
void ath_handle_rx_intr(struct ath_softc *sc);
int ath_rx_init(struct ath_softc *sc, int nbufs);
void ath_rx_cleanup(struct ath_softc *sc);
int ath_rx_tasklet(struct ath_softc *sc, int flush);
int ath_rx_input(struct ath_softc *sc,
		 struct ath_node *node,
		 int is_ampdu,
		 struct sk_buff *skb,
		 struct ath_recv_status *rx_status,
		 enum ATH_RX_TYPE *status);
int ath__rx_indicate(struct ath_softc *sc,
		    struct sk_buff *skb,
		    struct ath_recv_status *status,
		    u_int16_t keyix);
int ath_rx_subframe(struct ath_node *an, struct sk_buff *skb,
		    struct ath_recv_status *status);

/******/
/* TX */
/******/

#define ATH_FRAG_PER_MSDU       1
#define ATH_TXBUF               (512/ATH_FRAG_PER_MSDU)
/* max number of transmit attempts (tries) */
#define ATH_TXMAXTRY            13
/* max number of 11n transmit attempts (tries) */
#define ATH_11N_TXMAXTRY        10
/* max number of tries for management and control frames */
#define ATH_MGT_TXMAXTRY        4
#define WME_BA_BMP_SIZE         64
#define WME_MAX_BA              WME_BA_BMP_SIZE
#define ATH_TID_MAX_BUFS        (2 * WME_MAX_BA)
#define TID_TO_WME_AC(_tid)				\
	((((_tid) == 0) || ((_tid) == 3)) ? WME_AC_BE :	\
	 (((_tid) == 1) || ((_tid) == 2)) ? WME_AC_BK :	\
	 (((_tid) == 4) || ((_tid) == 5)) ? WME_AC_VI :	\
	 WME_AC_VO)


/* Wireless Multimedia Extension Defines */
#define WME_AC_BE               0 /* best effort */
#define WME_AC_BK               1 /* background */
#define WME_AC_VI               2 /* video */
#define WME_AC_VO               3 /* voice */
#define WME_NUM_AC              4

enum ATH_SM_PWRSAV{
	ATH_SM_ENABLE,
	ATH_SM_PWRSAV_STATIC,
	ATH_SM_PWRSAV_DYNAMIC,
};

/*
 * Data transmit queue state.  One of these exists for each
 * hardware transmit queue.  Packets sent to us from above
 * are assigned to queues based on their priority.  Not all
 * devices support a complete set of hardware transmit queues.
 * For those devices the array sc_ac2q will map multiple
 * priorities to fewer hardware queues (typically all to one
 * hardware queue).
 */
struct ath_txq {
	u_int			axq_qnum;	/* hardware q number */
	u_int32_t		*axq_link;	/* link ptr in last TX desc */
	struct list_head	axq_q;		/* transmit queue */
	spinlock_t		axq_lock;	/* lock on q and link */
	unsigned long		axq_lockflags;	/* intr state when must cli */
	u_int			axq_depth;	/* queue depth */
	u_int8_t                axq_aggr_depth; /* aggregates queued */
	u_int32_t		axq_totalqueued;/* total ever queued */
	u_int			axq_intrcnt;	/* count to determine
						if descriptor should generate
						int on this txq. */
	bool			stopped;	/* Is mac80211 queue
						stopped ? */
	/* State for patching up CTS when bursting */
	struct	ath_buf		*axq_linkbuf;	/* virtual addr of last buffer*/
	struct	ath_desc	*axq_lastdsWithCTS;	/* first desc of the
					last descriptor that contains CTS  */
	struct	ath_desc	*axq_gatingds; /* final desc of the gating desc
				* that determines whether lastdsWithCTS has
				* been DMA'ed or not */
	struct list_head	axq_acq;
};

/* per TID aggregate tx state for a destination */
struct ath_atx_tid {
	struct list_head	list;	/* round-robin tid entry */
	struct list_head	buf_q;    /* pending buffers */
	struct ath_node         *an;        /* parent node structure */
	struct ath_atx_ac       *ac;        /* parent access category */
	struct ath_buf          *tx_buf[ATH_TID_MAX_BUFS];/* active tx frames */
	u_int16_t               seq_start;  /* starting seq of BA window */
	u_int16_t               seq_next;   /* next seq to be used */
	u_int16_t               baw_size;   /* BA window size */
	int                     tidno;      /* TID number */
	int                     baw_head;   /* first un-acked tx buffer */
	int                     baw_tail;   /* next unused tx buffer slot */
	int                     sched;      /* TID is scheduled */
	int                     paused;     /* TID is paused */
	int                     cleanup_inprogress; /* aggr of this TID is
						being teared down */
	u_int32_t               addba_exchangecomplete:1; /* ADDBA state */
	int32_t                 addba_exchangeinprogress;
	int                     addba_exchangeattempts;
};

/* per access-category aggregate tx state for a destination */
struct ath_atx_ac {
	int                     sched;      /* dest-ac is scheduled */
	int                     qnum; /* H/W queue number associated
					with this AC */
	struct list_head	list;   /* round-robin txq entry */
	struct list_head	tid_q;      /* queue of TIDs with buffers */
};

/* per dest tx state */
struct ath_atx {
	struct ath_atx_tid  tid[WME_NUM_TID];
	struct ath_atx_ac   ac[WME_NUM_AC];
};

/* per-frame tx control block */
struct ath_tx_control {
	struct ath_node *an;	/* destination to sent to */
	int if_id;		/* only valid for cab traffic */
	int qnum;		/* h/w queue number */
	u_int ht:1;             /* if it can be transmitted using HT */
	u_int ps:1;             /* if one or more stations are in PS mode */
	u_int use_minrate:1;	/* if this frame should transmitted using
				minimum rate */
	enum hal_pkt_type atype;	/* Atheros packet type */
	enum hal_key_type keytype;	/* key type */
	u_int flags;		/* HAL flags */
	u_int16_t seqno;	/* sequence number */
	u_int16_t tidno;	/* tid number */
	u_int16_t txpower;	/* transmit power */
	u_int16_t frmlen;       /* frame length */
	u_int32_t keyix;        /* key index */
	int min_rate;		/* minimum rate */
	int mcast_rate;		/* multicast rate */
	u_int16_t nextfraglen;	/* next fragment length */
	/* below is set only by ath_dev */
	struct ath_softc *dev;	/* device handle */
	dma_addr_t dmacontext;
};

/* per frame tx status block */
struct ath_xmit_status {
	int retries; /* number of retries to successufully
			transmit this frame */
	int flags; /* status of transmit */
#define ATH_TX_ERROR        0x01
#define ATH_TX_XRETRY       0x02
#define ATH_TX_BAR          0x04
};

struct ath_tx_stat {
	int rssi;		/* RSSI (noise floor ajusted) */
	int rssictl[ATH_MAX_ANTENNA];	/* RSSI (noise floor ajusted) */
	int rssiextn[ATH_MAX_ANTENNA];	/* RSSI (noise floor ajusted) */
	int rateieee;		/* data rate xmitted (IEEE rate code) */
	int rateKbps;		/* data rate xmitted (Kbps) */
	int ratecode;		/* phy rate code */
	int flags;		/* validity flags */
/* if any of ctl,extn chain rssis are valid */
#define ATH_TX_CHAIN_RSSI_VALID 0x01
/* if extn chain rssis are valid */
#define ATH_TX_RSSI_EXTN_VALID  0x02
	u_int32_t airtime;	/* time on air per final tx rate */
};

struct ath_txq *ath_txq_setup(struct ath_softc *sc, int qtype, int subtype);
void ath_tx_cleanupq(struct ath_softc *sc, struct ath_txq *txq);
int ath_tx_setup(struct ath_softc *sc, int haltype);
void ath_draintxq(struct ath_softc *sc, enum hal_bool retry_tx);
void ath_tx_draintxq(struct ath_softc *sc,
	struct ath_txq *txq, enum hal_bool retry_tx);
void ath_tx_node_init(struct ath_softc *sc, struct ath_node *an);
void ath_tx_node_cleanup(struct ath_softc *sc,
	struct ath_node *an, bool bh_flag);
void ath_tx_node_free(struct ath_softc *sc, struct ath_node *an);
void ath_txq_schedule(struct ath_softc *sc, struct ath_txq *txq);
int ath_tx_init(struct ath_softc *sc, int nbufs);
int ath_tx_cleanup(struct ath_softc *sc);
int ath_tx_get_qnum(struct ath_softc *sc, int qtype, int haltype);
int ath_txq_update(struct ath_softc *sc, int qnum, struct hal_txq_info *q);
int ath_tx_start(struct ath_softc *sc, struct sk_buff *skb);
void ath_tx_tasklet(struct ath_softc *sc);
u_int32_t ath_txq_depth(struct ath_softc *sc, int qnum);
u_int32_t ath_txq_aggr_depth(struct ath_softc *sc, int qnum);
void ath_notify_txq_status(struct ath_softc *sc, u_int16_t queue_depth);
void ath_tx_complete(struct ath_softc *sc, struct sk_buff *skb,
		     struct ath_xmit_status *tx_status, struct ath_node *an);

/**********************/
/* Node / Aggregation */
/**********************/

/* indicates the node is clened up */
#define ATH_NODE_CLEAN          0x1
/* indicates the node is 80211 power save */
#define ATH_NODE_PWRSAVE        0x2

#define ADDBA_TIMEOUT              200 /* 200 milliseconds */
#define ADDBA_EXCHANGE_ATTEMPTS    10
#define ATH_AGGR_DELIM_SZ          4   /* delimiter size   */
#define ATH_AGGR_MINPLEN           256 /* in bytes, minimum packet length */
/* number of delimiters for encryption padding */
#define ATH_AGGR_ENCRYPTDELIM      10
/* minimum h/w qdepth to be sustained to maximize aggregation */
#define ATH_AGGR_MIN_QDEPTH        2
#define ATH_AMPDU_SUBFRAME_DEFAULT 32
#define IEEE80211_SEQ_SEQ_SHIFT    4
#define IEEE80211_SEQ_MAX          4096
#define IEEE80211_MIN_AMPDU_BUF    0x8

/* return whether a bit at index _n in bitmap _bm is set
 * _sz is the size of the bitmap  */
#define ATH_BA_ISSET(_bm, _n)  (((_n) < (WME_BA_BMP_SIZE)) &&		\
				((_bm)[(_n) >> 5] & (1 << ((_n) & 31))))

/* return block-ack bitmap index given sequence and starting sequence */
#define ATH_BA_INDEX(_st, _seq) (((_seq) - (_st)) & (IEEE80211_SEQ_MAX - 1))

/* returns delimiter padding required given the packet length */
#define ATH_AGGR_GET_NDELIM(_len)					\
	(((((_len) + ATH_AGGR_DELIM_SZ) < ATH_AGGR_MINPLEN) ?           \
	  (ATH_AGGR_MINPLEN - (_len) - ATH_AGGR_DELIM_SZ) : 0) >> 2)

#define BAW_WITHIN(_start, _bawsz, _seqno) \
	((((_seqno) - (_start)) & 4095) < (_bawsz))

#define ATH_DS_BA_SEQ(_ds)               ((_ds)->ds_us.tx.ts_seqnum)
#define ATH_DS_BA_BITMAP(_ds)            (&(_ds)->ds_us.tx.ba_low)
#define ATH_DS_TX_BA(_ds)                ((_ds)->ds_us.tx.ts_flags & HAL_TX_BA)
#define ATH_AN_2_TID(_an, _tidno)        (&(_an)->an_aggr.tx.tid[(_tidno)])

enum ATH_AGGR_STATUS {
	ATH_AGGR_DONE,
	ATH_AGGR_BAW_CLOSED,
	ATH_AGGR_LIMITED,
	ATH_AGGR_SHORTPKT,
	ATH_AGGR_8K_LIMITED,
};

enum ATH_AGGR_CHECK {
	AGGR_NOT_REQUIRED,
	AGGR_REQUIRED,
	AGGR_CLEANUP_PROGRESS,
	AGGR_EXCHANGE_PROGRESS,
	AGGR_EXCHANGE_DONE
};

struct aggr_rifs_param {
	int param_max_frames;
	int param_max_len;
	int param_rl;
	int param_al;
	struct ath_rc_series *param_rcs;
};

/* Per-node aggregation state */
struct ath_node_aggr {
	struct ath_atx      tx;         /* node transmit state */
	struct ath_arx      rx;         /* node receive state */
};

/* driver-specific node state */
struct ath_node {
	struct list_head	list;
	struct ath_softc    	*an_sc; 		/* back pointer */
	atomic_t		an_refcnt;
	struct ath_chainmask_sel an_chainmask_sel;
	struct ath_node_aggr 	an_aggr; /* A-MPDU aggregation state */
	u_int8_t             	an_smmode; /* SM Power save mode */
	u_int8_t         	an_flags;
	u8	 		an_addr[ETH_ALEN];
};

void ath_tx_resume_tid(struct ath_softc *sc,
	struct ath_atx_tid *tid);
enum ATH_AGGR_CHECK ath_tx_aggr_check(struct ath_softc *sc,
	struct ath_node *an, u8 tidno);
void ath_tx_aggr_teardown(struct ath_softc *sc,
	struct ath_node *an, u_int8_t tidno);
void ath_rx_aggr_teardown(struct ath_softc *sc,
	struct ath_node *an, u_int8_t tidno);
int ath_rx_aggr_start(struct ath_softc *sc,
		      const u8 *addr,
		      u16 tid,
		      u16 *ssn);
int ath_rx_aggr_stop(struct ath_softc *sc,
		     const u8 *addr,
		     u16 tid);
int ath_tx_aggr_start(struct ath_softc *sc,
		      const u8 *addr,
		      u16 tid,
		      u16 *ssn);
int ath_tx_aggr_stop(struct ath_softc *sc,
		     const u8 *addr,
		     u16 tid);
void ath_newassoc(struct ath_softc *sc,
	struct ath_node *node, int isnew, int isuapsd);
struct ath_node *ath_node_attach(struct ath_softc *sc,
	u_int8_t addr[ETH_ALEN], int if_id);
void ath_node_detach(struct ath_softc *sc, struct ath_node *an, bool bh_flag);
struct ath_node *ath_node_get(struct ath_softc *sc, u_int8_t addr[ETH_ALEN]);
void ath_node_put(struct ath_softc *sc, struct ath_node *an, bool bh_flag);
struct ath_node *ath_node_find(struct ath_softc *sc, u_int8_t *addr);

/*******************/
/* Beacon Handling */
/*******************/

/*
 * Regardless of the number of beacons we stagger, (i.e. regardless of the
 * number of BSSIDs) if a given beacon does not go out even after waiting this
 * number of beacon intervals, the game's up.
 */
#define BSTUCK_THRESH           	(9 * ATH_BCBUF)
#define	ATH_BCBUF               	4   /* number of beacon buffers */
#define ATH_DEFAULT_BINTVAL     	100 /* default beacon interval in TU */
#define ATH_DEFAULT_BMISS_LIMIT 	10
#define	ATH_BEACON_AIFS_DEFAULT		0  /* Default aifs for ap beacon q */
#define	ATH_BEACON_CWMIN_DEFAULT	0  /* Default cwmin for ap beacon q */
#define	ATH_BEACON_CWMAX_DEFAULT	0  /* Default cwmax for ap beacon q */
#define IEEE80211_MS_TO_TU(x)           (((x) * 1000) / 1024)

/* beacon configuration */
struct ath_beacon_config {
	u_int16_t beacon_interval;
	u_int16_t listen_interval;
	u_int16_t dtim_period;
	u_int16_t bmiss_timeout;
	u_int8_t dtim_count;
	u_int8_t tim_offset;
	union {
		u_int64_t last_tsf;
		u_int8_t last_tstamp[8];
	} u; /* last received beacon/probe response timestamp of this BSS. */
};

/* offsets in a beacon frame for
 * quick acess of beacon content by low-level driver */
struct ath_beacon_offset {
	u_int8_t *bo_tim;	/* start of atim/dtim */
};

void ath9k_beacon_tasklet(unsigned long data);
void ath_beacon_config(struct ath_softc *sc, int if_id);
int ath_beaconq_setup(struct ath_hal *ah);
int ath_beacon_alloc(struct ath_softc *sc, int if_id);
void ath_bstuck_process(struct ath_softc *sc);
void ath_beacon_tasklet(struct ath_softc *sc, int *needmark);
void ath_beacon_free(struct ath_softc *sc);
void ath_beacon_return(struct ath_softc *sc, struct ath_vap *avp);
void ath_beacon_sync(struct ath_softc *sc, int if_id);
void ath_update_beacon_info(struct ath_softc *sc, int avgbrssi);
void ath_get_beaconconfig(struct ath_softc *sc,
			  int if_id,
			  struct ath_beacon_config *conf);
struct sk_buff *ath_get_beacon(struct ath_softc *sc,
			       int if_id,
			       struct ath_beacon_offset *bo,
			       struct ath_tx_control *txctl);
int ath_update_beacon(struct ath_softc *sc,
		      int if_id,
		      struct ath_beacon_offset *bo,
		      struct sk_buff *skb,
		      int mcast);
/********/
/* VAPs */
/********/

#define ATH_IF_HW_OFF           0x0001	/* hardware state needs to turn off */
#define ATH_IF_HW_ON            0x0002	/* hardware state needs to turn on */
/* STA only: the associated AP is HT capable */
#define ATH_IF_HT               0x0004
/* AP/IBSS only: current BSS has privacy on */
#define ATH_IF_PRIVACY          0x0008
#define ATH_IF_BEACON_ENABLE    0x0010	/* AP/IBSS only: enable beacon */
#define ATH_IF_BEACON_SYNC      0x0020	/* IBSS only: need to sync beacon */

/*
 * Define the scheme that we select MAC address for multiple
 * BSS on the same radio. The very first VAP will just use the MAC
 * address from the EEPROM. For the next 3 VAPs, we set the
 * U/L bit (bit 1) in MAC address, and use the next two bits as the
 * index of the VAP.
 */

#define ATH_SET_VAP_BSSID_MASK(bssid_mask) \
	((bssid_mask)[0] &= ~(((ATH_BCBUF-1)<<2)|0x02))

/* VAP configuration (from protocol layer) */
struct ath_vap_config {
	u_int32_t av_fixed_rateset;
	u_int32_t av_fixed_retryset;
};

/* driver-specific vap state */
struct ath_vap {
	struct ieee80211_vif            *av_if_data; /* interface(vap)
				instance from 802.11 protocal layer */
	enum hal_opmode                 av_opmode;  /* VAP operational mode */
	struct ath_buf                  *av_bcbuf;  /* beacon buffer */
	struct ath_beacon_offset        av_boff;    /* dynamic update state */
	struct ath_tx_control           av_btxctl;  /* tx control information
							for beacon */
	int                             av_bslot;   /* beacon slot index */
	struct ath_txq                  av_mcastq;  /* multicast
						transmit queue */
	struct ath_vap_config           av_config;  /* vap configuration
					parameters from 802.11 protocol layer*/
};

int ath_vap_attach(struct ath_softc *sc,
		   int if_id,
		   struct ieee80211_vif *if_data,
		   enum hal_opmode opmode,
		   enum hal_opmode iv_opmode,
		   int nostabeacons);
int ath_vap_detach(struct ath_softc *sc, int if_id);
int ath_vap_config(struct ath_softc *sc,
	int if_id, struct ath_vap_config *if_config);
int ath_vap_down(struct ath_softc *sc, int if_id, u_int flags);
int ath_vap_listen(struct ath_softc *sc, int if_id);
int ath_vap_join(struct ath_softc *sc,
		 int if_id,
		 const u_int8_t bssid[ETH_ALEN],
		 u_int flags);
int ath_vap_up(struct ath_softc *sc,
	       int if_id,
	       const u_int8_t bssid[ETH_ALEN],
	       u_int8_t aid,
	       u_int flags);

/*********************/
/* Antenna diversity */
/*********************/

#define ATH_ANT_DIV_MAX_CFG      2
#define ATH_ANT_DIV_MIN_IDLE_US  1000000  /* us */
#define ATH_ANT_DIV_MIN_SCAN_US  50000	  /* us */

enum ATH_ANT_DIV_STATE{
	ATH_ANT_DIV_IDLE,
	ATH_ANT_DIV_SCAN,	/* evaluating antenna */
};

struct ath_antdiv {
	struct ath_softc *antdiv_sc;
	u_int8_t antdiv_start;
	enum ATH_ANT_DIV_STATE antdiv_state;
	u_int8_t antdiv_num_antcfg;
	u_int8_t antdiv_curcfg;
	u_int8_t antdiv_bestcfg;
	int32_t antdivf_rssitrig;
	int32_t antdiv_lastbrssi[ATH_ANT_DIV_MAX_CFG];
	u_int64_t antdiv_lastbtsf[ATH_ANT_DIV_MAX_CFG];
	u_int64_t antdiv_laststatetsf;
	u_int8_t antdiv_bssid[ETH_ALEN];
};

void ath_slow_ant_div_init(struct ath_antdiv *antdiv,
	struct ath_softc *sc, int32_t rssitrig);
void ath_slow_ant_div_start(struct ath_antdiv *antdiv,
			    u_int8_t num_antcfg,
			    const u_int8_t *bssid);
void ath_slow_ant_div_stop(struct ath_antdiv *antdiv);
void ath_slow_ant_div(struct ath_antdiv *antdiv,
		      struct ieee80211_hdr *wh,
		      struct ath_rx_status *rx_stats);
void ath_setdefantenna(void *sc, u_int antenna);

/********************/
/* Main driver core */
/********************/

/*
 * Default cache line size, in bytes.
 * Used when PCI device not fully initialized by bootrom/BIOS
*/
#define DEFAULT_CACHELINE       32
#define	ATH_DEFAULT_NOISE_FLOOR -95
#define ATH_REGCLASSIDS_MAX     10
#define ATH_CABQ_READY_TIME     80  /* % of beacon interval */
#define ATH_PREAMBLE_SHORT	(1<<0)
#define ATH_PROTECT_ENABLE	(1<<1)
#define ATH_MAX_SW_RETRIES      10
/* Num farmes difference in tx to flip default recv */
#define	ATH_ANTENNA_DIFF	2
#define ATH_CHAN_MAX            255
#define IEEE80211_WEP_NKID      4       /* number of key ids */
#define IEEE80211_RATE_VAL      0x7f
/*
 * The key cache is used for h/w cipher state and also for
 * tracking station state such as the current tx antenna.
 * We also setup a mapping table between key cache slot indices
 * and station state to short-circuit node lookups on rx.
 * Different parts have different size key caches.  We handle
 * up to ATH_KEYMAX entries (could dynamically allocate state).
 */
#define	ATH_KEYMAX	        128        /* max key cache size we handle */
#define	ATH_KEYBYTES	        (ATH_KEYMAX/NBBY) /* storage space in bytes */

#define RESET_RETRY_TXQ         0x00000001
#define ATH_IF_ID_ANY   	0xff

#define ATH_TXPOWER_MAX         100     /* .5 dBm units */

#define ATH_ISR_NOSCHED         0x0000	/* Do not schedule bottom half */
/* Schedule the bottom half for execution */
#define ATH_ISR_SCHED           0x0001
/* This was not my interrupt, for shared IRQ's */
#define ATH_ISR_NOTMINE         0x0002

#define RSSI_LPF_THRESHOLD         -20
#define ATH_RSSI_EP_MULTIPLIER     (1<<7)  /* pow2 to optimize out * and / */
#define ATH_RATE_DUMMY_MARKER      0
#define ATH_RSSI_LPF_LEN           10
#define ATH_RSSI_DUMMY_MARKER      0x127

#define ATH_EP_MUL(x, mul)         ((x) * (mul))
#define ATH_EP_RND(x, mul)						\
	((((x)%(mul)) >= ((mul)/2)) ? ((x) + ((mul) - 1)) / (mul) : (x)/(mul))
#define ATH_RSSI_OUT(x)							\
	(((x) != ATH_RSSI_DUMMY_MARKER) ?				\
	 (ATH_EP_RND((x), ATH_RSSI_EP_MULTIPLIER)) : ATH_RSSI_DUMMY_MARKER)
#define ATH_RSSI_IN(x)					\
	(ATH_EP_MUL((x), ATH_RSSI_EP_MULTIPLIER))
#define ATH_LPF_RSSI(x, y, len)						\
	((x != ATH_RSSI_DUMMY_MARKER) ? \
		(((x) * ((len) - 1) + (y)) / (len)) : (y))
#define ATH_RSSI_LPF(x, y) do {						\
		if ((y) >= RSSI_LPF_THRESHOLD)				\
			x = ATH_LPF_RSSI((x), \
				ATH_RSSI_IN((y)), ATH_RSSI_LPF_LEN); \
	} while (0)


enum PROT_MODE {
	PROT_M_NONE = 0,
	PROT_M_RTSCTS,
	PROT_M_CTSONLY
};

enum ieee80211_clist_cmd {
	CLIST_UPDATE,
	CLIST_DFS_UPDATE,
	CLIST_NEW_COUNTRY
};

enum RATE_TYPE {
	NORMAL_RATE = 0,
	HALF_RATE,
	QUARTER_RATE
};

struct ath_ht_info {
	enum hal_ht_macmode tx_chan_width;
	u_int16_t maxampdu;
	u_int8_t mpdudensity;
	u_int8_t ext_chan_offset;
};

struct ath_softc {
	struct ieee80211_hw *hw; /* mac80211 instance */
	struct pci_dev		*pdev;	    /* Bus handle */
	void __iomem		*mem;	    /* address of the device */
	struct tasklet_struct   intr_tq;    /* General tasklet */
	struct tasklet_struct   bcon_tasklet; /* Beacon tasklet */
	struct ath_config       sc_config;  /* per-instance load-time
						parameters */
	int                     sc_debug;   /* Debug masks */
	struct ath_hal          *sc_ah;     /* HAL Instance */
	struct ath_rate_softc    *sc_rc;     /* tx rate control support */
	u_int32_t               sc_intrstatus; /* HAL_STATUS */
	enum hal_opmode         sc_opmode;  /* current operating mode */

	/* Properties, Config */
	unsigned int
		sc_invalid             : 1, /* being detached */
		sc_mrretry             : 1, /* multi-rate retry support */
		sc_needmib             : 1, /* enable MIB stats intr */
		sc_hasdiversity        : 1, /* rx diversity available */
		sc_diversity           : 1, /* enable rx diversity */
		sc_hasveol             : 1, /* tx VEOL support */
		sc_beacons             : 1, /* beacons running */
		sc_hasbmask            : 1, /* bssid mask support */
		sc_hastsfadd           : 1, /* tsf adjust support */
		sc_scanning            : 1, /* scanning active */
		sc_nostabeacons        : 1, /* no beacons for station */
		sc_hasclrkey           : 1, /* CLR key supported */
		sc_stagbeacons         : 1, /* use staggered beacons */
		sc_txaggr              : 1, /* enable 11n tx aggregation */
		sc_rxaggr              : 1, /* enable 11n rx aggregation */
		sc_hasautosleep        : 1, /* automatic sleep after TIM */
		sc_waitbeacon          : 1, /* waiting for first beacon
						after waking up */
		sc_no_tx_3_chains      : 1, /* user, hardware, regulatory
					or country may disallow transmit on
					three chains. */
		sc_update_chainmask    : 1, /* change chain mask */
		sc_rx_chainmask_detect : 1, /* enable rx chain mask detection */
		sc_rx_chainmask_start  : 1, /* start rx chain mask detection */
		sc_hashtsupport        : 1, /* supports 11n */
		sc_full_reset          : 1, /* force full reset */
		sc_slowAntDiv          : 1; /* enable slow antenna diversity */
	enum wireless_mode      sc_curmode;     /* current phy mode */
	u_int16_t               sc_curtxpow;    /* current tx power limit */
	u_int16_t               sc_curaid;      /* current association id */
	u_int8_t                sc_curbssid[ETH_ALEN];
	u_int8_t                sc_myaddr[ETH_ALEN];
	enum PROT_MODE          sc_protmode;    /* protection mode */
	u_int8_t                sc_mcastantenna;/* Multicast antenna number */
	u_int8_t                sc_txantenna;   /* data tx antenna
						(fixed or auto) */
	u_int8_t                sc_nbcnvaps;    /* # of vaps sending beacons */
	u_int16_t               sc_nvaps;       /* # of active virtual ap's */
	struct ath_vap          *sc_vaps[ATH_BCBUF]; /* interface id
						to avp map */
	enum hal_int            sc_imask;       /* interrupt mask copy */
	u_int8_t                sc_bssidmask[ETH_ALEN];
	u_int8_t                sc_defant;      /* current default antenna */
	u_int8_t                sc_rxotherant;  /* rx's on non-default antenna*/
	u_int16_t               sc_cachelsz;    /* cache line size */
	int                     sc_slotupdate;  /* slot to next advance fsm */
	int                     sc_slottime;    /* slot time */
	u_int8_t                sc_noreset;
	int                     sc_bslot[ATH_BCBUF];/* beacon xmit slots */
	struct hal_node_stats   sc_halstats;    /* station-mode rssi stats */
	struct list_head        node_list;
	struct ath_ht_info      sc_ht_info;
	int16_t                 sc_noise_floor; /* signal noise floor in dBm */
	enum hal_ht_extprotspacing   sc_ht_extprotspacing;
	u_int8_t                sc_tx_chainmask;
	u_int8_t                sc_rx_chainmask;
	u_int8_t                sc_rxchaindetect_ref;
	u_int8_t                sc_rxchaindetect_thresh5GHz;
	u_int8_t                sc_rxchaindetect_thresh2GHz;
	u_int8_t                sc_rxchaindetect_delta5GHz;
	u_int8_t                sc_rxchaindetect_delta2GHz;
	u_int32_t               sc_rtsaggrlimit; /* Chipset specific
						aggr limit */
	u32                     sc_flags;
#ifdef CONFIG_SLOW_ANT_DIV
	/* Slow antenna diversity */
	struct ath_antdiv       sc_antdiv;
#endif
	enum {
		OK,                 /* no change needed */
		UPDATE,             /* update pending */
		COMMIT              /* beacon sent, commit change */
	} sc_updateslot;            /* slot time update fsm */

	/* Crypto */
	u_int                   sc_keymax;      /* size of key cache */
	u_int8_t                sc_keymap[ATH_KEYBYTES];/* key use bit map */
	u_int8_t		sc_splitmic;	/* split TKIP MIC keys */
	int                     sc_keytype;     /* type of the key being used */

	/* RX */
	struct list_head	sc_rxbuf;       /* receive buffer */
	struct ath_descdma      sc_rxdma;       /* RX descriptors */
	int                     sc_rxbufsize;   /* rx size based on mtu */
	u_int32_t               *sc_rxlink;     /* link ptr in last RX desc */
	u_int32_t               sc_rxflush;     /* rx flush in progress */
	u_int64_t               sc_lastrx;      /* tsf of last rx'd frame */

	/* TX */
	struct list_head	sc_txbuf;       /* transmit buffer */
	struct ath_txq          sc_txq[HAL_NUM_TX_QUEUES];
	struct ath_descdma      sc_txdma;       /* TX descriptors */
	u_int                   sc_txqsetup;    /* h/w queues setup */
	u_int                   sc_txintrperiod;/* tx interrupt batching */
	int                     sc_haltype2q[HAL_WME_AC_VO+1]; /* HAL WME
							AC -> h/w qnum */
	u_int32_t               sc_ant_tx[8];   /* recent tx frames/antenna */

	/* Beacon */
	struct hal_txq_info     sc_beacon_qi;   /* adhoc only: beacon
						queue parameters */
	struct ath_descdma      sc_bdma;        /* beacon descriptors */
	struct ath_txq          *sc_cabq;       /* tx q for cab frames */
	struct list_head	sc_bbuf;	/* beacon buffers */
	u_int                   sc_bhalq;       /* HAL q for outgoing beacons */
	u_int                   sc_bmisscount;  /* missed beacon transmits */
	u_int32_t               ast_be_xmit; /* beacons transmitted */

	/* Rate */
	struct ieee80211_rate          rates[IEEE80211_NUM_BANDS][ATH_RATE_MAX];
	const struct hal_rate_table    *sc_rates[WIRELESS_MODE_MAX];
	const struct hal_rate_table    *sc_currates;   /* current rate table */
	u_int8_t                       sc_rixmap[256]; /* IEEE to h/w
						rate table ix */
	u_int8_t                       sc_minrateix;   /* min h/w rate index */
	u_int8_t                       sc_protrix; /* protection rate index */
	struct {
		u_int32_t rateKbps;      /* transfer rate in kbs */
		u_int8_t ieeerate;       /* IEEE rate */
	} sc_hwmap[256];         /* h/w rate ix mappings */

	/* Channel, Band */
	struct ieee80211_channel channels[IEEE80211_NUM_BANDS][ATH_CHAN_MAX];
	struct ieee80211_supported_band sbands[IEEE80211_NUM_BANDS];
	struct hal_channel              sc_curchan; /* current h/w channel */

	/* Locks */
	spinlock_t              sc_rxflushlock; /* lock of RX flush */
	spinlock_t              sc_rxbuflock;   /* rxbuf lock */
	spinlock_t              sc_txbuflock;   /* txbuf lock */
	spinlock_t              sc_resetlock;
	spinlock_t              node_lock;
};

int ath_init(u_int16_t devid, struct ath_softc *sc);
void ath_deinit(struct ath_softc *sc);
int ath_open(struct ath_softc *sc, struct hal_channel *initial_chan);
int ath_suspend(struct ath_softc *sc);
int ath_intr(struct ath_softc *sc);
int ath_reset(struct ath_softc *sc);
void ath_scan_start(struct ath_softc *sc);
void ath_scan_end(struct ath_softc *sc);
int ath_set_channel(struct ath_softc *sc, struct hal_channel *hchan);
void ath_setup_channel_list(struct ath_softc *sc,
			    enum ieee80211_clist_cmd cmd,
			    const struct hal_channel *chans,
			    int nchan,
			    const u_int8_t *regclassids,
			    u_int nregclass,
			    int countrycode);
void ath_setup_rate(struct ath_softc *sc,
		    enum wireless_mode wMode,
		    enum RATE_TYPE type,
		    const struct hal_rate_table *rt);

/*********************/
/* Utility Functions */
/*********************/

void ath_set_macmode(struct ath_softc *sc, enum hal_ht_macmode macmode);
void ath_key_reset(struct ath_softc *sc, u_int16_t keyix, int freeslot);
int ath_keyset(struct ath_softc *sc,
	       u_int16_t keyix,
	       struct hal_keyval *hk,
	       const u_int8_t mac[ETH_ALEN]);
int ath_get_hal_qnum(u16 queue, struct ath_softc *sc);
int ath_get_mac80211_qnum(u_int queue, struct ath_softc *sc);
void ath_setslottime(struct ath_softc *sc);
void ath_update_txpow(struct ath_softc *sc);
int ath_cabq_update(struct ath_softc *);
void ath_get_currentCountry(struct ath_softc *sc,
	struct hal_country_entry *ctry);
u_int64_t ath_extend_tsf(struct ath_softc *sc, u_int32_t rstamp);
void ath_internal_reset(struct ath_softc *sc);
u_int32_t ath_chan2flags(struct ieee80211_channel *chan, struct ath_softc *sc);
dma_addr_t ath_skb_map_single(struct ath_softc *sc,
			      struct sk_buff *skb,
			      int direction,
			      dma_addr_t *pa);
void ath_skb_unmap_single(struct ath_softc *sc,
			  struct sk_buff *skb,
			  int direction,
			  dma_addr_t *pa);
void ath_mcast_merge(struct ath_softc *sc, u_int32_t mfilt[2]);
enum hal_ht_macmode ath_cwm_macmode(struct ath_softc *sc);

#endif /* CORE_H */
