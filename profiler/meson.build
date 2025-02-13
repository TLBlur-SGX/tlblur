profiler_common = static_library('profiler_common',
  dependencies : [sgx_urts, sgx_uae_service],
  link_with : [test_common, sgx_step], 
  include_directories : ['../enclaves/common', '../sgx-step'],
  install : true,
  c_args : c_args,
)

foreach e : built_enclaves
  shared_library('prof-' + e['name'], 
    e['name'] + '.c',
    e['untrusted_edl'],
    link_with : [e['untrusted_lib'], profiler_common], 
    link_whole : [test_common],
    dependencies : [sgx_urts, sgx_uae_service, sgx_usgxssl],
    include_directories : ['../enclaves/common', '../sgx-step'],
    install : true,
    c_args : c_args,
  )
endforeach
