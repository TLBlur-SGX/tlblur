enclaves += {
  'name': 'simple',
  'edl': files('Enclave/encl.edl'),
  'config': files('Enclave/encl.config.xml'),
  'trusted_sources': files('Enclave/encl.c', 'Enclave/asm.S') + trusted_common_files,
  'untrusted_sources': files('main.c'),
  'include': include_directories('Enclave'),
}
