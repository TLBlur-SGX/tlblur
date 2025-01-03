subdir('Enclave/yescrypt')

enclaves += {
  'name': 'yescrypt',
  'edl': files('Enclave/encl.edl'),
  'config': files('Enclave/encl.config.xml'),
  'trusted_sources': yescrypt_sources
    + files('Enclave/encl.c')
    + trusted_common_files,
  'untrusted_sources': files('main.c'),
  'include': include_directories('Enclave', 'Enclave/yescrypt'),
}
