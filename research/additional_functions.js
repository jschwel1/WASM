function runAndDraw(){
	var start = new Date().getTime();
	var r = Module._getPolygons(10);
	var arr = Module["HEAP32"].subarray(r/4, r/4+20000);
	var ctx = document.querySelector("canvas").getContext("2d");
	ctx.clearRect(0, 0, 500, 500);
	ctx.beginPath()
	ctx.moveTo(arr[0]*5, arr[1]*5);
	var sptx = arr[0];
	var spty = arr[1];
	for (var i = 2; i < arr.length-1; i+=2){
		if (Math_abs(arr[i]-arr[i-2]) > 2 || Math_abs(arr[i+1]-arr[i-1]) > 2){
				ctx.moveTo(arr[i]*5, arr[i+1]*5);
		}
		else
			ctx.lineTo(arr[i]*5, arr[i+1]*5);
	}
	ctx.fill();
	ctx.closePath();
	var end = new Date().getTime();
	console.log(end-start + " ms");
}
