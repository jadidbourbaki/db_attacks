project('db_attacks', 'cpp', default_options: ['cpp_std=c++17'])

rocksdb_dep = dependency('rocksdb', required: true)
leveldb_dep = dependency('leveldb', required: true)
openssl_dep = dependency('openssl', required: true)

executable(
    'experiment_1_rocksdb',
    'experiment_1_rocksdb.cpp',
    dependencies: [rocksdb_dep, openssl_dep],
)

executable(
    'experiment_1_leveldb',
    'experiment_1_leveldb.cpp',
    dependencies: [leveldb_dep, openssl_dep],
)

executable(
    'insertion_attack_leveldb',
    'insertion_attack_leveldb.cpp',
    dependencies: [leveldb_dep],
)

executable(
    'experiment_4_leveldb',
    'experiment_4_inserts_with_prp.cpp',
    dependencies: [leveldb_dep, openssl_dep],
)

executable(
    'experiment_4_rocksdb',
    'experiment_4_inserts_with_prp_rocksdb.cpp',
    dependencies: [rocksdb_dep, openssl_dep],
)