/**
*
*/
function getCType(type){
	var cType={};
	if ((type == 'double')) {
		cType.size = 8;
		cType.heap = "HEAPF64";
	}
	else if ((type == 'float')) {
		cType.size = 4;
		cType.heap = "HEAPF32";
	}
	else if ((type == 'int') || (type == 'int32')){
		cType.size = 4;
		cType.heap = "HEAP32"
	}
	else if ((type == 'uint32')) {
		cType.size = 4;
		cType.head = "HEAPU32";
	}
	else if (type == 'int8' || (type == 'char')){
		cType.size = 1;
		cType.heap = "HEAP8"
	}
	else if ((type == 'uint8')) {
		cType.size = 1;
		cType.head = "HEAPU8";
	}
	else {
		console.log("Could not build cType for: ", type);
		throw new Error("Invalid type: " + type);
	}
	return cType;
}
/**
* Puts an array into the webassembly heap and returns a pointer to it
* arr - the array
* type - the c-type (char, int, uint, float, double)
*/
function arrayToPtr(arr, type){
	var cType = getCType(type);
	var offset = Module._malloc(arr.length*cType.size);
	Module[cType.heap].set(arr, offset/cType.size);
	return offset;
}

/**
* Takes a ptr and converts it to an array
* ptr - ptr to the array in memory
* arrLength - the length of the array
* type - the c-type of the array
*/
function ptrToArray(ptr, arrLength, type){
	var cType = getCType(type);
	if (cType == null) {
		console.log("No cType made for ", type);
	}
	var result = Module[cType.heap].subarray(
						ptr/cType.size,
						ptr/cType.size + arrLength);
						
	Module._free(ptr);
	return result;
}

/**
* Puts a JS string in the HEAP for a WebAssembly function to use 
*
*/
function stringToPtr(string){
	var ptr = Module.allocate(intArrayFromString(string), 'i8', ALLOC_NORMAL);
	return ptr;
}

function ptrToString(ptr){
	var str = Pointer_stringify(ptr);
	Module._free(ptr);
	return str;
}


/*function buildList(){
	var arr = [];
	Module._addNode(headPtr, 213);
	for (var i = 0; i < 1000; i++) {
		arr.push(i);
	}
	var arr = new Int32Array(arr)
	var arrPtr = arrayToPtr(arr, 'int32');
	Module._addNodes(headPtr, arrPtr, arr.length);
}*/

function setupCreaturesN(n){
	var headPtr = Module._createCreature();
	var crs = [];
	crs.push(Module._replicate(headPtr));
	for (var i = 0; i < n; i++) {
		crs.push(Module._replicate(crs[i]));
	}
	return headPtr;
}

function setupCreatures(){
	var headPtr = Module._createCreature();
	var crs = [];
	crs.push(Module._replicate(headPtr));
	for (var i = 0; i < 10; i++) {
		crs.push(Module._replicate(crs[i]));
	}
	return headPtr;
}

function getCreatureJSON(creature, strlen){
	var ptr = Module._malloc(strlen);
	var strPtr = Module._getCreatureJSON(headPtr, ptr);
	var output = ptrToString(strPtr);
	
	return output;
}

function timer(fxn){
	var start = new Date().getTime();
	for (var arg in arguments){
		arguments[arg]();
	}
	var end = new Date().getTime();
	return (end-start);
}

function testJSONTiming(){
	var str;
	var time;
	var output = "";
	var trials = 100000;
	for (var len = 1000; len < trials; len += 1000){
		var avg = 0.0;
		if (len % 10000 === 0)
			console.log(100.0*len/trials + "% done");
		for (var round = 0; round < 10; round++){
			time = timer(function(){
				str = ptrToString(Module._buildJSON(len));
			});
			avg += time;
		}
		avg/=10;
		Module._free(str);
		output += len + ' ' + avg  + '\n';
	}
	console.log(output);
}

function testBuildObjTiming(){
	var time;
	var output = "";
	var trials = 100000;
	var headPtr;
	var rounds = 1;
	for (var len = 1000; len < trials; len += 1000){
		var avg = 0.0;
		if (len % 10000 === 0)
			console.log(100.0*len/trials + "% done");
		headPtr = setupCreaturesN(len);
		for (var round = 0; round < rounds; round++){
			time = timer(function(){
				buildCrObjR(headPtr);
			});
			avg += time;
		}
		avg/=rounds;
		Module._free(headPtr);
		output += len + ' ' + avg  + '\n';
	}
	console.log(output);
}

function buildCrObj(crPtr){
	return {
		nrg: Module["HEAPF32"][crPtr/4],
		xloc: Module["HEAPF32"][crPtr/4 + 1],
		yloc: Module["HEAPF32"][crPtr/4 + 2],
		zloc: Module["HEAPF32"][crPtr/4 + 3],
		cId: Module["HEAP32"][crPtr/4 + 4],
		type: Module["HEAP8"][crPtr + 20],
		child: Module["HEAP32"][crPtr/4 + 6]
	};
}

function buildCrObjR(crPtr){
	var arr = [];
	var crObj = buildCrObj(crPtr);
	while (crObj.child != 0 && crObj.child != undefined){
		arr.push(crObj);
		crObj = buildCrObj(crObj.child);
	}
	arr.push(crObj);
	return arr;
}
