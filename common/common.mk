CANOPEN_SRC   = $(PROJ_SRC)/CANopenNode
CO_STACK      = $(CANOPEN_SRC)/stack

PROJINC       = $(PROJ_SRC)/include             \
                $(CANOPEN_SRC)                  \
                $(CO_STACK)

PROJSRC       = $(PROJ_SRC)/can_hw.c            \
                $(PROJ_SRC)/can_threads.c       \
                $(PROJ_SRC)/can.c               \
                $(PROJ_SRC)/CO_driver.c         \
                $(CO_STACK)/CO_SDO.c            \
                $(CO_STACK)/CO_Emergency.c      \
                $(CO_STACK)/CO_NMT_Heartbeat.c  \
                $(CO_STACK)/CO_SYNC.c           \
                $(CO_STACK)/CO_PDO.c            \
                $(CO_STACK)/CO_HBconsumer.c     \
                $(CO_STACK)/CO_SDOmaster.c      \
                $(CO_STACK)/CO_LSSmaster.c      \
                $(CO_STACK)/CO_LSSslave.c       \
                $(CO_STACK)/CO_trace.c          \
                $(CANOPEN_SRC)/CANopen.c        \
                $(PROJ_SRC)/oresat.c


# Shared variables.
ALLCSRC      += $(PROJSRC)
ALLINC       += $(PROJINC)
