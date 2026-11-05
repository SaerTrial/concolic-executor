if not fs.is_file(meson.project_source_root() / 'hooks/core.c')
  error('Missing firmware hooks. Run: git submodule update --init hooks')
endif

specific_ss.add(when: ['CONFIG_CONCOLIC_MACHINE', 'TARGET_ARM'], if_true: files(
  'utils/mem.c',
  'configurable_machine.c',
  'hook_port.c',
  'arm/interrupt_firing.c',
  '../../hooks/core.c',
  '../../hooks/hooks.c',
  '../../hooks/arm/abi.c',
))
