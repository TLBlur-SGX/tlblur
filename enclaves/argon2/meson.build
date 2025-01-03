subdir('Enclave/argon2')

enclaves += {
  'name': 'argon2',
  'edl': files('Enclave/encl.edl'),
  'config': files('Enclave/encl.config.xml'),
  'trusted_sources': argon2_sources
    + files('Enclave/encl.c')
    + trusted_common_files,
  'untrusted_sources': files('main.c'),
  'include': include_directories('Enclave', 'Enclave/argon2', 'Enclave/argon2/include', 'Enclave/argon2/src'),
}
