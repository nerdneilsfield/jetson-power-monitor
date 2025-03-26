.PHONY: help
help:
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?##/ {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

.PHONY: build
build: ## Build the project
	cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON
	cmake --build build -j $(nproc)

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
test-python: install-python-dev ## Run the tests
	python -m pytest tests

.PHONY: test-rust
test-rust: ## Run the tests
	cd bindings/rust && cargo test

.PHONY: example-rust
example-rust: ## Run the examples
	cd bindings/rust && cargo run --example cpu_monitor


	