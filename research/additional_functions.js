function runAndDraw(val){
    if (!val) val = 10;
	var start = new Date().getTime();
	var r = Module._getPolygons(val);
	var arr = Module["HEAP32"].subarray(r/4, r/4+20000);
	var ctx = document.querySelector("canvas").getContext("2d");
	ctx.fillStyle = "rgb("+Math.floor(255*val/30)+", "
						  +Math.floor(255*val/30)+", "
						  +Math.floor(255*val/30)+")";
//	ctx.clearRect(0, 0, 500, 500);
	ctx.beginPath();
	ctx.moveTo(arr[0]*5, arr[1]*5);
	var sptx = arr[0];
	var spty = arr[1];
	for (var i = 2; i < arr.length-1; i+=2){
		if (Math_abs(arr[i]-arr[i-2]) > 2 || Math_abs(arr[i+1]-arr[i-1]) > 2){
				ctx.closePath();
				ctx.stroke();
				ctx.beginPath();
				ctx.moveTo(arr[i]*5, arr[i+1]*5);
		}
		else
			ctx.lineTo(arr[i]*5, arr[i+1]*5);
	}
	ctx.closePath();
	ctx.stroke();
	var end = new Date().getTime();
	console.log(end-start + " ms");
}

var level = 1;
var last = null;
var fr = 0;
var sum = 0;
var count = 0;
function startAnimation(fps){
    level = 0;
    fr = 1000/fps;
    
    sum = 0;
    count = 0;
 	var ctx = document.querySelector("canvas").getContext("2d");
   	ctx.clearRect(0, 0, 500, 500);
	
    requestAnimationFrame(animate);
}

function animate(time){
    if (last === null) last = time;
    
    if (time-last > fr){
        level+=(time-last)*.01;
   	 	runAndDraw(level);
       //console.log(1000/(time-last) + " fps");
        sum += 1000/(time-last);
        count++;
        last = time;
    }
    if (level < 30){
        requestAnimationFrame(animate);
    }
    
}
