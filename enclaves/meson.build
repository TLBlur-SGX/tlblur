subdir('common')
subdir('tcommon')

enclaves = []

subdir('simple')
subdir('micro')
subdir('rsa')
subdir('libjpeg')
subdir('yescrypt')
subdir('argon2')
subdir('wolfssl')
subdir('openssl')

built_enclaves = []

foreach e : enclaves
  inc = ['./common', '../prefetcher'] + e['include']

  # NOTE: order of dependencies actually matters, sgx_trts must come last!
  dep = [sgx_tstdc, sgx_tservice, sgx_tcrypto]

  if e.has_key('dependencies')
    dep += e['dependencies']
  endif

  tlblur_deps = dep
  base_deps = dep

  if e.has_key('tlblur_deps')
    tlblur_deps += e['tlblur_deps']
    # tlblur_deps += e['base_deps']
  endif

  if e.has_key('base_deps')
    base_deps += e['base_deps']
  endif

  tlblur_deps += [sgx_trts, sgx_t]
  base_deps += [sgx_trts, sgx_t]

  partial_base_deps = []
  foreach d : base_deps
    partial_base_deps += d.partial_dependency(includes : true)
  endforeach
  partial_tlblur_deps = []
  foreach d : tlblur_deps
    partial_tlblur_deps += d.partial_dependency(includes : true)
  endforeach
  
  trusted_edl = edger_gen_trusted.process(e['edl'])
  trusted_edl_header = edger_gen_trusted_header.process(e['edl'])
  edl_lib = static_library('encl-' + e['name'] + '-edl', trusted_edl, 
    c_args : trusted_c_args,
    dependencies: sgx_tstdc.partial_dependency(includes : true),
    pic: true,
  )
  link = [tlblur_runtime, edl_lib]  

  to_harden = []
  to_harden_large_pages = []
  to_harden_jump_targets = []
  to_sign = []

  app_static_orig = static_library('encl-' + e['name'] + '-app',
    trusted_edl_header,
    e['trusted_sources'],
    include_directories : inc,
    dependencies : partial_base_deps,
    c_args : trusted_c_args,
    pic : true,
    install : true,
  )

  app_static = custom_target(
    'encl-' + e['name'] + '-app-moved-sec',
    output: 'encl-' + e['name'] + '-app-moved-sec.a',
    input: [app_static_orig],
    command: [objcopy] + ['--rename-section', '.text=.tlblur.text', '@INPUT@', '@OUTPUT@'],
  )

  app_static_instrumented_orig = static_library('encl-' + e['name'] + '-app-instr',
    trusted_edl_header,
    e['trusted_sources'],
    include_directories : inc,
    dependencies : partial_tlblur_deps,
    c_args : trusted_c_args + tlblur_instrumentation_args,
    pic : true,
    install : true,
  )

  app_static_instrumented = custom_target(
    'encl-' + e['name'] + '-app-moved-sec-instr',
    output: 'encl-' + e['name'] + '-app-moved-sec-instr.a',
    input: [app_static_instrumented_orig],
    command: [objcopy] + ['--rename-section', '.text=.tlblur.text', '@INPUT@', '@OUTPUT@'],
  )

  trusted = shared_library('encl-' + e['name'],
    include_directories : inc,
    link_with : link + [app_static],
    link_args : trusted_link_args,
    dependencies : base_deps,
    c_args : trusted_c_args,
  )
  to_sign += { 'name': trusted.name(), 'so': trusted }

  trusted_relocs = shared_library('encl-' + e['name'] + '-relocs',
    include_directories : inc,
    link_with : link + [app_static],
    link_args : trusted_link_args + ['-Wl,--emit-relocs'],
    dependencies : base_deps,
    c_args : trusted_c_args,
  )
  to_harden += trusted_relocs

  trusted_instrumented = shared_library('encl-' + e['name'] + '-instrumented',
    include_directories : inc,
    link_with : link + [app_static_instrumented],
    link_args : trusted_link_args,
    dependencies : tlblur_deps,
    c_args : trusted_c_args + tlblur_instrumentation_args,
  )
  to_sign += { 'name': trusted_instrumented.name(), 'so': trusted_instrumented }

  trusted_instrumented_relocs = shared_library('encl-' + e['name'] + '-instrumented-relocs',
    include_directories : inc,
    link_with : link + [app_static_instrumented],
    link_args : trusted_link_args + ['-Wl,--emit-relocs'],
    dependencies : tlblur_deps,
    c_args : trusted_c_args + tlblur_instrumentation_args,
  )
  to_harden += trusted_instrumented_relocs

  foreach t : to_harden
    name = t.name() + '-bolt-base'
    trusted_hardened = custom_target(
      name,
      output: name + '.so',
      input: [t],
      command: [bolt] + bolt_args + ['-o', '@OUTPUT@', '@INPUT@'],
    )
    to_sign += { 'name': name, 'so': trusted_hardened }
  endforeach

  foreach t : to_harden
    name = t.name() + '-bolt'
    trusted_hardened = custom_target(
      name,
      output: name + '.so',
      input: [t],
      command: [bolt] + bolt_args + '--tlblur' + ['-o', '@OUTPUT@', '@INPUT@'],
    )
    to_sign += { 'name': name, 'so': trusted_hardened }
  endforeach

  foreach t : to_harden
    name = t.name() + '-bolt-16KiB'
    trusted_hardened = custom_target(
      name,
      output: name + '.so',
      input: [t],
      command: [bolt] + bolt_args 
        + ['--tlblur', '--tlblur-page-mask=0x3fff'] 
        + ['-o', '@OUTPUT@', '@INPUT@'],
    )
    to_sign += { 'name': name, 'so': trusted_hardened }
  endforeach

  foreach t : to_harden
    name = t.name() + '-bolt-jmp-tgts-eflags'
    trusted_hardened = custom_target(
      name,
      output: name + '.so',
      input: [t],
      command: [bolt] + bolt_args + [
        '--tlblur',
        '--tlblur-instrument-jump-targets',
        '--tlblur-store-eflags=true', 
      ] + ['-o', '@OUTPUT@', '@INPUT@'],
    )
    to_sign += { 'name': name, 'so': trusted_hardened }
  endforeach

  foreach t : to_harden
    name = t.name() + '-bolt-jmp-tgts'
    trusted_hardened = custom_target(
      name,
      output: name + '.so',
      input: [t],
      command: [bolt] + bolt_args + [
        '--tlblur',
        '--tlblur-instrument-jump-targets',
        '--tlblur-store-eflags=false', 
      ] + ['-o', '@OUTPUT@', '@INPUT@'],
    )
    to_sign += { 'name': name, 'so': trusted_hardened }
  endforeach

  foreach t : to_harden
    name = t.name() + '-bolt-opt'
    trusted_hardened = custom_target(
      name,
      output: name + '.so',
      input: [t],
      command: [bolt] + bolt_args + [
        '--tlblur',
        '--tlblur-instrument-jump-targets',
        '--tlblur-store-eflags=false', 
        '--tlblur-page-mask=0x3fff',
      ] + ['-o', '@OUTPUT@', '@INPUT@'],
    )
    to_sign += { 'name': name, 'so': trusted_hardened }
  endforeach

  foreach t : to_sign
    trusted_signed = custom_target(
        's-' + t['name'],
        output : 's-' + t['name'] + '.so',
        input : [t['so'], private_key] + e['config'],
        command : [sign, 'sign', 
          '-key', '@INPUT1@', 
          '-enclave', '@INPUT0@', 
          '-out', '@OUTPUT@', 
          '-config', '@INPUT2@'
        ],
        install_dir : get_option('libdir'),
        install : true,
    )
  endforeach

  untrusted_edl = edger_gen_untrusted.process(e['edl'])
  e += { 'untrusted_edl': untrusted_edl }

  untrusted = static_library('test-' + e['name'] + '-untrusted', 
    untrusted_edl,
    include_directories : ['./common'] + e['include'],
    dependencies : [sgx_t],
    pic : true,
    c_args : untrusted_c_args,
  )

  e += { 'untrusted_lib': untrusted }

  exe = executable('test-' + e['name'], e['untrusted_sources'], 
    untrusted_edl,
    link_with : [test_common, untrusted, sgx_step], 
    dependencies : [sgx_urts, sgx_uae_service, sgx_usgxssl],
    include_directories : ['./common', '../sgx-step'],
    install : true,
    c_args : c_args,
  )

  built_enclaves += e
endforeach

