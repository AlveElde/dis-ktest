#include <rdma/ib_verbs.h>

#define STATUS_START    "Started.\n"
#define STATUS_COMPLETE "Completed.\n"
#define STATUS_FAIL     "Failed.\n"

#define TOTAL_PD    1
#define TOTAL_CQ    1
#define TOTAL_QP    1
#define TOTAL_SGE   1

struct pd_ctx {
    struct ib_pd        *ibpd;
    struct ib_device    *ibdev;
    int                 flags;
};

struct cq_ctx {
    struct ib_cq        *ibcq;
    struct ib_device    *ibdev;
    void (*comp_handler)(struct ib_cq *ibcq, void *cq_context);
    void (*event_handler)(struct ib_event *ibevent, void *cq_context);
    void *context;
    struct ib_cq_init_attr attr;
};

struct qp_ctx {
    struct ib_qp            *ibqp;
    struct ib_pd            *ibpd;
    struct ib_qp_init_attr  attr;
};

// struct mr_ctx {
//     struct ib_mr    *ibmr;
//     struct ib_pd    *ibpd;
// };

struct send_wr_ctx {
    struct ib_send_wr   *ibwr;
    struct ib_send_wr   *ibbadwr;
    struct ib_qp        *ibqp;
    struct ib_sge       *ibsge;
    enum ib_wr_opcode	opcode;
    int                 num_sge;
    int			        send_flags;
    u64		            wr_id;
};

struct sge_ctx {
    struct ib_sge   *ibsge;
	u64	            addr;
	u32	            length;
	u32	            lkey;
};

int test_requester(struct ib_device *ibdev);

int test_responder(struct ib_device *ibdev);