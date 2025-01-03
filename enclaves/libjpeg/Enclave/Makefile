# CC		  = gcc
# AR		  = ar
# LD		  = gcc
MAKEFLAGS += --no-builtin-rules
EDGER		  = sgx_edger8r
SIGNER		  = sgx_sign
ROOT = $(realpath ../../../)
INCLUDE           = -I$(SGX_SDK)/include/ -I$(SGX_SDK)/include/tlibc -I$(ROOT)/prefetcher -I$(ROOT)/test/common
T_CFLAGS	  = $(CFLAGS) -nostdinc -fvisibility=hidden -fpie -fstack-protector -g -Os
U_CFLAGS	  = $(CFLAGS) -nostdinc -fvisibility=hidden -fpie -fstack-protector -g
AR_FLAGS	  = rcs
OBJECTS		  = encl.o

ifeq ($(SIM), 1)
    LIB_SGX_TRTS      = -lsgx_trts_sim
    LIB_SGX_TSERVICE  = -lsgx_tservice_sim
else
    LIB_SGX_TRTS      = -lsgx_trts
    LIB_SGX_TSERVICE  = -lsgx_tservice
endif

ifeq ($(M32), 1)
	T_CFLAGS += -m32 -msse2 -DM32=1
	U_CFLAGS += -m32 -msse2
	LD_FLAGS  = -m32
else
	LIB_SUFX  = 64
endif

ENCLAVE_LIBS      = $(LIB_SGX_TRTS)
ENCLAVE_LIB_PARTS = -lsgx_tstdc -lsgx_tcrypto $(LIB_SGX_TSERVICE)
ENCLAVE	          = encl
PRIVATE_KEY       = private_key.pem
PUBLIC_KEY        = public_key.pem
KEY_SIZE          = 3072
ENCLAVE_EDL       = $(ENCLAVE).edl
ENCLAVE_CONFIG    = $(ENCLAVE).config.xml
OUTPUT_T          = test-libjpeg-$(ENCLAVE).so
OUTPUT_T_UNSIG    = test-libjpeg-$(ENCLAVE).unsigned.so
LIB_DIRS          = -L $(SGX_SDK)/lib$(LIB_SUFX)/
LD_FLAGS         += -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles \
                    -Wl,--whole-archive -Wl,--start-group $(ENCLAVE_LIBS) -Wl,--end-group \
                    -Wl,--no-whole-archive -Wl,--start-group $(ENCLAVE_LIB_PARTS) -Wl,--end-group \
                    -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
                    -Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
                    -Wl,--defsym,__ImageBase=0
TRUSTED_OBJECTS   = $(ENCLAVE)_t.o ../../../runtime/tlblur.o
TRUSTED_CODE      = $(ENCLAVE)_t.h $(ENCLAVE)_t.c

JPEG_SOURCES = jaricom.c jcomapi.c jutils.c jerror.c jmemmgr.c jmemnobs.c \
 		jcapimin.c jcapistd.c jcarith.c jctrans.c jcparam.c \
        jcinit.c jcmaster.c jcmarker.c jcmainct.c jcprepct.c \
        jccoefct.c jccolor.c jcsample.c jchuff.c jcdctmgr.c jfdctfst.c \
        jfdctflt.c jfdctint.c \
		jdapimin.c jdapistd.c jdarith.c jdtrans.c \
        jdmaster.c jdinput.c jdmarker.c jdhuff.c jdmainct.c jdatadst.c jdatasrc.c \
        jdcoefct.c jdpostct.c jddctmgr.c jidctfst.c jidctflt.c \
        jidctint.c jdsample.c jdcolor.c jquant1.c jquant2.c jdmerge.c

JPEG_C := $(addprefix jpeg-9e/, $(JPEG_SOURCES))
JPEG_ASM := $(JPEG_C:.c=.S)
JPEG_OBJECTS := $(JPEG_C:.c=.o)

#.SILENT:
all: $(OUTPUT_T)

# libjpeg:
# 	$(MAKE) -C jpeg-9e

$(OUTPUT_T) : $(TRUSTED_OBJECTS) $(OBJECTS) $(JPEG_OBJECTS) $(PRIVATE_KEY)
	clang $(OBJECTS) $(TRUSTED_OBJECTS) $(JPEG_OBJECTS) $(LD_FLAGS) $(LIB_DIRS) -o $(OUTPUT_T_UNSIG) 
	$(SIGNER) sign -key $(PRIVATE_KEY) -enclave $(OUTPUT_T_UNSIG) -out $(OUTPUT_T) -config $(ENCLAVE_CONFIG) > /dev/null 2> /dev/null
	
%_t.s : %_t.c edger
	clang -S $(INCLUDE) $(T_CFLAGS) $< -o $@

%.s : %.c
	clang -S $(INCLUDE) $(T_CFLAGS) $< -o $@

%.o : %.s
	clang $(INCLUDE) $(T_CFLAGS) -c $< -o $@

edger: $(ENCLAVE_EDL)
	$(EDGER) $(ENCLAVE_EDL)

.PHONY: scrub
scrub: 
	$(RM) $(PRIVATE_KEY) $(PUBLIC_KEY)

$(PRIVATE_KEY): 
	# generate 3072 bit private RSA key
	openssl genrsa -out $(PRIVATE_KEY) -3 $(KEY_SIZE)
	# extract public key
	openssl rsa -in $(PRIVATE_KEY) -pubout -out $(PUBLIC_KEY) 
	
.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(JPEG_OBJECTS) $(OUTPUT_T_UNSIG) $(OUTPUT_T) $(OUTPUT_U)
	$(RM) $(TRUSTED_OBJECTS) $(TRUSTED_CODE)
