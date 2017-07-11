#!/bin/bash

cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_rw.c   -o nvme_rw
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W   -c -o nvme_identify.o nvme_identify.c
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_smart.c nvme_identify.o   -o nvme_smart
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_set_feature.c   -o nvme_set_feature
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_get_feature.c   -o nvme_get_feature
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_id_ns.c nvme_identify.o   -o nvme_id_ns
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_id_ctrl.c nvme_identify.o   -o nvme_id_ctrl
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_format_ns.c nvme_identify.o   -o nvme_format_ns
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_async.c   -o nvme_async
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_read_regs.c   -o nvme_read_regs
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_irqcoal.c   -o nvme_irqcoal
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_fw_download.c   -o nvme_fw_download
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_fw_activate.c   -o nvme_fw_activate
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_rw_test.c   -o nvme_rw_test
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_rw_test_bw.c   -o nvme_rw_test_bw -lrt
cc -m64 -O2 -g -pthread -D_GNU_SOURCE -D_REENTRANT -W  -m64 -lm  nvme_rw_test_16k.c   -o nvme_rw_test_16k -lrt
