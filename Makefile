.PHONY: build
build: setup
	meson compile -C build

.PHONY: dependencies
dependencies:
	brew install leveldb rocksdb openssl

.PHONY: setup
setup:
	meson setup build

.PHONY: clean
clean:
	rm -rf build
	rm results/*.pdf

.PHONY: run-leveldb
run-leveldb:
	./build/experiment_1_leveldb

.PHONY: run-rocksdb
run-rocksdb: 
	./build/experiment_1_rocksdb

.PHONY: run-insertion-attack-leveldb
run-insertion-attack-leveldb:
	./build/insertion_attack_leveldb

.PHONY: run-inserts-leveldb
run-inserts-leveldb:
	./build/experiment_4_leveldb

.PHONY: run-inserts-rocksdb
run-inserts-rocksdb:
	./build/experiment_4_rocksdb