files=main.c

build: $(files)
	emcc -Wall -s WASM=1 -s NO_EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -o index.js -O3 $(files)

buildc: $(files)
	gcc -g -Wall -Ofast -D NOWASM -o output $(files) -lm
	cp $(files)  ./backups

buildcd: $(files)
	gcc -g -Wall -Og -D NOWASM -o output -D DEBUGGING $(files) -lm
	cp $(files)  ./backups

emrun: build index.wasm index.js index.html
	emrun --port=8080 --serve_after_exit --no_browser index.html &

pythonRun: $(files) index.wasm index.js index.html
	python -m SimpleHTTPServer 8080 &

clean: 
	-rm index.wasm
	-rm index.js

