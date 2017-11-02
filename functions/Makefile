
build: function.c
	emcc -s WASM=1 -s NO_EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -o index.js -O3 function.c

emrun: build index.wasm index.js index.html
	emrun --port=8080 --serve_after_exit --no_browser index.html &

pythonRun: function.c index.wasm index.js index.html
	python -m SimpleHTTPServer 8080 &

clean: 
	-rm index.wasm
	-rm index.js

