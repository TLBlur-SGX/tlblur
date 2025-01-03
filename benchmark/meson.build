foreach e : built_enclaves
  shared_library('bench-' + e['name'], 
    'lib/common.c',
    'lib/' + e['name'] + '.c',
    e['untrusted_edl'],
    link_with : [test_common, e['untrusted_lib'], sgx_step], 
    dependencies : [sgx_urts, sgx_uae_service, sgx_usgxssl],
    include_directories : ['../enclaves/common', '../sgx-step', './include'],
    install : true,
    c_args : c_args,
  )
endforeach
