#ifndef _DNVME_DS_H_
#define _DNVME_DS_H_

#include <linux/list.h>
#include "dnvme_interface.h"

#define    NVME_DS_VERSION    1.1

/*
 * Structure to define a global list of interrupt vector and
 * associated completion queue id. All the completion queue id's
 * created must have an associated interrupt vector which needs to
 * be populated in this isr tracking structure. The association of
 * is either 1 -- 1 or 1 -- * (INTVEC -- CQ ID's).
 */
struct isr_track {
    struct    list_head    isr_list_hd;   /* List head of isr tracker    */
    u16       int_vec;                     /* Interrupt vector            */
    u16       cq_id[];                    /* List of associated CQ's     */
};

/*
 * Strucutre used to define all the essential parameters
 * related to PRP1, PRP2 and PRP List
 */
struct nvme_prps {
    u32 npages; /* No. of pages inside the PRP List */
    u32 type; /* refers to types of PRP Possible */
    /* List of virtual pointers to PRP List pages */
    __le64 **vir_prp_list;
    __le64 prp1; /* Physical address in PRP1 of command */
    __le64 prp2; /* Physical address in PRP2 of command */
    /* TODO: Will be removed once complete IOCTL is working */
    dma_addr_t first_dma; /* First entry in PRP List */
};

/*
 * structure for the CQ tracking params with virtual address and size.
 */
struct nvme_trk_cq {
    u8          *vir_kern_addr; /* phy addr ptr to the q's alloc to kern mem*/
    dma_addr_t  acq_dma_addr;   /* dma mapped address using dma_alloc       */
    u32         size;           /* length in bytes of the alloc Q in kernel */
    u32 __iomem *dbs;           /* Door Bell stride                         */
    struct nvme_prps  prp_persist; /* PRP element in CQ                        */
    u8          contig;         /* Indicates if prp list is contig or not   */
};

/*
 *    Structure definition for tracking the commands.
 */
struct cmd_track {
    struct list_head    cmd_list_hd;  /* link-list using the kernel list    */
    u16    unique_id;    /* driver assigned unique id for a particuler cmd. */
    u16    persist_q_id; /* Q ID used for Create/Delete Queues */
    u8     opcode;       /* command opcode as per spec                      */
    enum   nvme_cmds   cmd_set;   /* what cmd set does this opcode belong to */
    struct nvme_prps prp_nonpersist; /* Non persistent PRP entries */
};

/*
 * structure definition for SQ tracking parameters.
 */
struct nvme_trk_sq {
    void        *vir_kern_addr; /* virtual kernal address using kmalloc */
    dma_addr_t  asq_dma_addr; /* dma mapped address using dma_alloc */
    u32         size; /* length in bytes of allocated Q in kernel */
    u16         unique_cmd_id; /* unique counter for each comand in SQ */
    u32 __iomem *dbs; /* Door Bell stride */
    struct nvme_prps  prp_persist; /* PRP element in CQ */
    u8          contig; /* Indicates if prp list is contig or not */
    /* TODO xxx remove once not needed */
    /* struct cmd_track    *cmd_track_list;  to track a particular cmd*/
    /* link-list for tracking commands in SQ */
    struct    list_head    sq_cmd_trk_ll;
};

/*
 * Structure with Metrics of CQ. Has a node which makes it work with
 * kernel linked lists.
 */
struct metrics_cq {
    struct    list_head    cq_list_hd; /* link-list using the kernel list  */
    struct    nvme_gen_cq  public_cq;  /* parameters in nvme_gen_cq        */
    struct    nvme_trk_cq  private_cq; /* parameters in nvme_trk_cq        */
};

/*
 * Structure with Metrics of SQ. Has a node which makes it work with
 * kernel linked lists.
 */
struct metrics_sq {
    struct    list_head    sq_list_hd;  /* link-list using the kernel list  */
    struct    nvme_gen_sq  public_sq;   /* parameters in nvme_gen_sq        */
    struct    nvme_trk_sq  private_sq;  /* parameters in nvme_trk_sq        */
};

struct nvme_device {
    struct pci_dev  *pdev;          /* Pointer to the device in PCI space  */
    struct nvme_ctrl_reg __iomem *nvme_ctrl_space; /* Pointer to reg space */
    u8  *bar_0_mapped;                /* Bar 0 IO re-mapped value            */
    struct device   *dmadev;        /* Pointer to the dma device from pdev */
    u8  device_no;                  /* Current device number               */
};

/*
 * Structure which defines the device list for all the data structures
 * that are defined.
 */
struct metrics_device_list {
    struct  list_head   metrics_device_hd; /* metrics linked list head    */
    struct  isr_track   *isr_track_list;   /* ISR with CQ tracking list   */
    struct  metrics_cq  *metrics_cq_list;  /* CQ linked list              */
    struct  metrics_sq  *metrics_sq_list;  /* SQ linked list              */
    struct  nvme_device *pnvme_device;     /* Pointer to this nvme device */
};

/* extern device metrics linked list for exporting to project files */
extern struct metrics_device_list *pmetrics_device_list;
extern struct metrics_driver g_metrics_drv;

/**
 * This function gives the device metrics when the user requests. This
 * routine works with Add Q's including Admin and IO.
 * Assumes user allocated buffer memory to copy accordingly.
 * @param get_q_metrics
 * @return metrics data if success else failure.
 */
int nvme_get_q_metrics(struct nvme_get_q_metrics *get_q_metrics);

#endif