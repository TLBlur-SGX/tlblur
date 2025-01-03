subdir('Enclave/jpeg-9e')

enclaves += {
  'name': 'libjpeg',
  'edl': files('Enclave/encl.edl'),
  'config': files('Enclave/encl.config.xml'),
  'trusted_sources': libjpeg_sources_common 
    + libjpeg_sources_compression 
    + libjpeg_sources_decompression
    + files('Enclave/encl.c', 'Enclave/asm.S')
    + trusted_common_files,
  'untrusted_sources': files('main.c'),
  'include': include_directories('Enclave'),
}
