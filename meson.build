project('tlblur', 'c')

cc = meson.get_compiler('c')

##################
# Locate SGX SDK #
##################

# Try the SGX_SDK option first
sgx_sdk = get_option('sgx_sdk')

fs = import('fs')
if not fs.is_dir(sgx_sdk)
  # Best effort use of pkg-config to find SDK path
  sgx_urts = dependency('libsgx_urts', method : 'pkg-config')
  sgx_sdk = sgx_urts.get_pkgconfig_variable('prefix')  
endif

assert(fs.is_dir(sgx_sdk), 'SGX SDK not found!')

sgx_include = include_directories(sgx_sdk / 'include')
sgx_lib = sgx_sdk / 'lib64'
sgx_include_tlibc = include_directories(sgx_sdk / 'include/tlibc')

sgx = declare_dependency(
  link_args : ['-L' + sgx_lib],
  include_directories : sgx_include,
)
sgx_t = declare_dependency(include_directories : sgx_include_tlibc, dependencies : [sgx])

sgx_sim = get_option('sgx_sim')
if sgx_sim
  sgx_urts = declare_dependency(link_args : '-lsgx_urts_sim', dependencies : [sgx])
  sgx_uae_service = declare_dependency(link_args : '-lsgx_uae_service_sim', dependencies : [sgx])
  sgx_tservice = declare_dependency(link_args : '-lsgx_tservice_sim', dependencies : [sgx_t])
  sgx_trts = declare_dependency(
    link_args : ['-Wl,--whole-archive', '-lsgx_trts_sim'], 
    dependencies : [sgx_t]
  )
else
  sgx_urts = declare_dependency(link_args : '-lsgx_urts', dependencies : [sgx])
  sgx_uae_service = declare_dependency(link_args : '-lsgx_uae_service', dependencies : [sgx])
  sgx_tservice = declare_dependency(link_args : '-lsgx_tservice', dependencies : [sgx_t])
  sgx_trts = declare_dependency(
    link_args : ['-Wl,--whole-archive', '-lsgx_trts'], 
    dependencies : [sgx_t]
  )
endif

sgx_tstdc = declare_dependency(link_args : '-lsgx_tstdc', dependencies : [sgx_t])
sgx_tcrypto = declare_dependency(link_args : '-lsgx_tcrypto', dependencies : [sgx_t])
sgx_pthread = declare_dependency(link_args : '-lsgx_pthread', dependencies : [sgx_t])
sgx_tcxx = declare_dependency(link_args : '-lsgx_tcxx', dependencies : [sgx_t])

# HACK: Use a different archive for the instrumented tlibc
sgx_tstdc_tlblur = declare_dependency(link_args : '-lsgx_tstdc_tlblur', dependencies : [sgx_t])

if not sgx_tstdc_tlblur.found()
  message('Instrumented tstdc not found, falling back to default tstdc!')
  sgx_tstdc_tlblur = sgx_tstdc
endif

sgx_tstdc_tlblur = sgx_tstdc

sgx_ssl = get_option('sgx_ssl')
sgx_ssl_path = sgx_ssl
sgx_ssl_include = include_directories(sgx_ssl / 'include')
sgx_ssl_lib = sgx_ssl / 'lib64'

sgx_ssl = declare_dependency(
  link_args : ['-L' + sgx_ssl_lib],
  include_directories : sgx_ssl_include,
)

sgx_usgxssl = declare_dependency(
  link_args : '-lsgx_usgxssl', 
  dependencies : [sgx_ssl]
)
sgx_tsgxssl = declare_dependency(
  link_args : ['-Wl,--whole-archive', '-lsgx_tsgxssl'], 
  dependencies : [sgx_ssl]
)
sgx_tsgxssl_crypto = declare_dependency(link_args : '-lsgx_tsgxssl_crypto', dependencies : [sgx_ssl])
sgx_tsgxssl_crypto_tlblur = declare_dependency(link_args : '-lsgx_tsgxssl_crypto_tlblur', dependencies : [sgx_ssl])

if not sgx_tsgxssl_crypto_tlblur.found()
  message('Instrumented tsgxssl_crypto not found, falling back to default!')
  sgx_tsgxssl_crypto_tlblur = sgx_tsgxssl_crypto
else
  message('Found TLBlur OpenSSL')
endif

bolt = find_program('bolt')
objcopy = find_program('llvm-objcopy')

######################
# Compiler arguments #
######################

c_args = [
  '-fno-stack-protector', '-fno-builtin', '-fno-jump-tables',
  '-fno-common', '-Wno-attributes', '-g', '-D_GNU_SOURCE', '-O0',
  '-march=native', '-maes',
]

if sgx_sim
  message('Building in simulation mode')
  c_args += ['-DSE_SIM']
endif

trusted_c_args = c_args + [
  # '-ffunction-sections',
  '-nostdinc', '-fpie',
  '-fstack-protector', '-g', '-Os'
]

# HACK: Make wolfssl compile
trusted_c_args += [
  '-DWOLFSSL_SGX', 
  # '-DWOLFSSL_AESNI', 
  '-Wno-implicit-function-declaration'
]

untrusted_c_args = c_args + [
  '-nostdinc', '-fpie',
  '-fstack-protector', '-g',
]

tlblur_instrumentation_args = [
  # '-mllvm', '-debug-only=x86-tlblur',
  '-mllvm', '-x86-tlblur-instrument',
  '-mllvm', '-x86-tlblur-inline',
  # '-mllvm', '-x86-tlblur-counter-register',
  '-mllvm', '-verify-machineinstrs',
]

trusted_link_args = [
  '-Wl,--no-undefined', '-nostdlib', '-nodefaultlibs', '-nostartfiles',
  '-Wl,-Bstatic', '-Wl,-Bsymbolic', '-Wl,--no-undefined',
  '-Wl,-eenclave_entry', '-Wl,--export-dynamic',
  '-Wl,--defsym,__ImageBase=0',
  '-lsgx_pthread',
  '-Xlinker', '-verbose',
  '-Xlinker', '-T', meson.source_root() / 'enclaves/tlblur.ld',
]

bolt_args = [
  '-update-debug-sections', '-v=2',
	'-eliminate-unreachable=0', '-strip-rep-ret=0', '-simplify-conditional-tail-calls=0',
	'-align-macro-fusion=none', '-conservative-instrumentation', 
  # '--debug-only=bolt-tlblur',
	'--relocs=true', '--no-threads', '--jump-tables=none', 
  '--align-functions=32', '--align-functions-max-bytes=32', '--use-compact-aligner=false',
  # '--tlblur-page-mask=0x3fff',
  # '--tlblur-instrument-jump-targets',
  # '--tlblur-store-eflags=false',
]


#########
# Edger #
#########

edger = find_program(sgx_sdk / 'bin/sgx_edger8r', sgx_sdk / 'bin/x64/sgx_edger8r', required : true)

edger_search_paths = [meson.source_root() / 'enclaves', sgx_ssl_path / 'include', sgx_sdk / 'include']
edger_search_args = []
foreach p : edger_search_paths
  edger_search_args += ['--search-path', p]
endforeach

edger_gen_trusted = generator(edger,
                output : ['@BASENAME@_t.c', '@BASENAME@_t.h'],
                arguments : ['--trusted-dir', '@BUILD_DIR@', '--trusted', '@INPUT@'] + edger_search_args)
edger_gen_trusted_header = generator(edger,
                output : ['@BASENAME@_t.h'],
                arguments : ['--trusted-dir', '@BUILD_DIR@', '--trusted', '@INPUT@', '--header-only'] + edger_search_args)
edger_gen_untrusted = generator(edger,
                output : ['@BASENAME@_u.c', '@BASENAME@_u.h'],
                arguments : ['--untrusted-dir', '@BUILD_DIR@', '--untrusted', '@INPUT@'] + edger_search_args)

########
# Sign #
########

sign = find_program(sgx_sdk / 'bin/sgx_sign', sgx_sdk / 'bin/x64/sgx_sign', required : true)

openssl = find_program('openssl', required : true)

private_key = custom_target(
  'private_key.pem',
  output : 'private_key.pem',
  input : [],
  command: [openssl, 'genrsa', '-out', '@OUTPUT@', '-3', '3072'],
)

public_key = custom_target(
  'public_key.pem',
  output : 'public_key.pem',
  input : [private_key],
  command: [openssl, 'rsa', '-in', '@INPUT@', '-pubout', '-out', '@OUTPUT@'],
)

sgx_step = static_library('sgx-step', 
  './sgx-step/libsgxstep/aep_trampoline.S',
  './sgx-step/libsgxstep/irq_entry.S',
  './sgx-step/libsgxstep/rtm.S',
  './sgx-step/libsgxstep/transient.S',
  './sgx-step/libsgxstep/apic.c',
  './sgx-step/libsgxstep/cpu.c',
  './sgx-step/libsgxstep/debug.c',
  './sgx-step/libsgxstep/enclave.c',
  './sgx-step/libsgxstep/file.c',
  './sgx-step/libsgxstep/foreshadow.c',
  './sgx-step/libsgxstep/gdt.c',
  './sgx-step/libsgxstep/idt.c',
  './sgx-step/libsgxstep/pt.c',
  './sgx-step/libsgxstep/sched.c',
  './sgx-step/libsgxstep/spy.c',
  './sgx-step/libsgxstep/simstep.c',
  c_args : c_args,
  pic : true,
)

subdir('runtime')
subdir('enclaves')
subdir('benchmark')
subdir('profiler')
