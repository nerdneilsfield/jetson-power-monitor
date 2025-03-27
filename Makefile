.PHONY: help
help:
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?##/ {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

.PHONY: build
build: ## Build the project
	cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON -DBUILD_TESTS=ON
	cmake --build build -j $(nproc)

.PHONY: build-debug
build-debug: ## Build the project in debug mode
	cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DBUILD_EXAMPLES=ON -DSHOW_ALL=ON -DBUILD_TESTS=ON
	cmake --build build -j $(nproc)

.PHONY: test-c
test-c: ## Run the tests
	cd build && ctest

.PHONY: install
install: ## Install the project
	cmake --install build

.PHONY: clean
clean: ## Clean the project
	rm -rf build
	rm -rf dist


.PHONY: build-wheel
build-wheel: ## Build the wheel
	python -m build --wheel --no-isolation

.PHONY: install-wheel
install-python: ## Install python test
	python -m pip install -e .

.PHONY: install-python-dev
install-python-dev: ## Install python dev
	python -m pip install -e .[dev]


.PHONY: test-python
test-python: ## Run the tests
	python tests/test.py

.PHONY: copy-rust
copy-rust: ## Copy the rust project
	cp -r include/jetpwmon/jetpwmon.h bindings/rust/vendor/include/jetpwmon/
	cp -r src/jetpwmon.c bindings/rust/vendor/src/
	cp -r LICENSE bindings/rust/
	cp -r README.md bindings/rust/

.PHONY: build-rust
build-rust: copy-rust ## Build the rust project
	cd bindings/rust && cargo build

.PHONY: test-rust
test-rust: ## Run the tests
	cd bindings/rust && cargo test

.PHONY: example-rust
example-rust: ## Run the examples
	cd bindings/rust && cargo run --example matrix_multiply

test: test-rust test-python test-c ## Run all tests 

.PHONY: clean-rust
clean-rust: ## Clean the rust project
	cd bindings/rust && cargo clean

.PHONY: dist-clean
dist-clean: clean-rust clean ## Clean the project
	rm -rf dist
	rm -rf build
	rm -rf wheelhouse
	rm -rf .pytest_cache
	rm -rf .mypy_cache
	rm -rf .ruff_cache
	rm -rf .cache
	find . -type f -name "*.pyc" -delete
	find . -type f -name "*.pyo" -delete
	find . -type f -name "*.pyd" -delete
	find . -type f -name "*.pyw" -delete
	find . -type f -name "*.pyz" -delete
	find . -type f -name "*.pywz" -delete
	find . -type f -name "*.so" -delete
	find . -type f -name "*.dylib" -delete
	find . -type f -name "*.dll" -delete
	find . -type f -name "*.lib" -delete
	find . -type f -name "*.a" -delete
	find . -type d -name "*.egg-info" -exec rm -rf {} +
	find . -type f -name "*.egg" -delete
	find . -type f -name "*.whl" -delete
	find . -type d -name "*.dist-info" -exec rm -rf {} +
	find . -type f -name "*.build-info" -exec rm -rf {} +
	find . -type d -name "*.pytest_cache" -exec rm -rf {} +
	 