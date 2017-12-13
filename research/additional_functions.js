var resolution = 5;
var scale = 2;
function runAndDraw(){
	var start = new Date().getTime();
	try {
		var r = Module._getPolygons();
	}
	catch (e) {
		console.error(e);
		return;
	}
	var arr = Module["HEAP32"].subarray(r/4, r/4+20000);
	
	var ctx = document.querySelector("canvas").getContext("2d");
	ctx.beginPath();
	ctx.moveTo(arr[0]*scale, arr[1]*scale);
	ctx.lineWidth=scale;
	var sptx = arr[0];
	var spty = arr[1];
	for (var i = 4; i < arr.length-1; i+=4){
	    var thisX = arr[i], thisY = arr[i+1];
	    if (thisX == 0 && thisY == 0){
	        i = arr.length;
	        continue;
	    }
		if (thisX == sptx && thisY == spty){
        	    var enc = arr[i+2];
        	    var fillVal = 255-Math.ceil(2.55*enc*resolution);
        	   // ctx.fillStyle = "rgb("+fillVal+", "+fillVal+", "+fillVal+")";
        	    ctx.strokeStyle = "rgb("+fillVal+", "+fillVal+", "+fillVal+")";
				ctx.closePath();
				// ctx.fill();
				ctx.stroke();
				sptx = null;
				spty = null;
		}
		else if (sptx == null || spty == null){
		    sptx = thisX;
		    spty = thisY;
			ctx.beginPath();
			ctx.moveTo(thisX*scale, thisY*scale);
		}
		else
			ctx.lineTo(thisX*scale, thisY*scale);
	}
	var enc = arr[i+2];
	var fillVal = Math.floor(255*enc/10);
	ctx.strokeStyle = "rgb("+fillVal+", "+fillVal+", "+fillVal+")";
	ctx.closePath();
	ctx.stroke();
	Module._free(r);
	var end = new Date().getTime();
// 	console.log(end-start + " ms");
    return (end-start);
}

function getAverage(){
    var ctx = document.querySelector("canvas").getContext("2d");
    var sum = 0;
    var timeArr = [];
    for (var i = 0; i < 500; i++){
        ctx.clearRect(0, 0, 500, 500);
        var t = runAndDraw();
        sum+=t;
        timeArr.push(t);
    }
    console.log(sum/500);
    return timeArr;
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
    var ctx = document.querySelector("canvas").getContext("2d");
   	ctx.clearRect(0, 0, 500, 500);
    if (time-last > fr){
        // level+=(time-last)*.01;
        level += 1;
        Module._diffuse();
   	 	runAndDraw();
       //console.log(1000/(time-last) + " fps");
        sum += 1000/(time-last);
        count++;
        last = time;
    }
    if (level < 30){
        requestAnimationFrame(animate);
    }
    else {
        console.log(done);
    }
    
}

function startAnimateDiffusion(iterations){
    level = iterations || 1;
    requestAnimationFrame(animateDiffusion);
}


var lastTime = 0;
function animateDiffusion(time){
    if (lastTime == null) lastTime = time;
	if (time > lastTime+30) {
		lastTime = time;
        var ctx = document.querySelector("canvas").getContext("2d");
        ctx.clearRect(0, 0, 500, 500); Module._diffuse(); runAndDraw();
        level--;
    
    }
	if (level > 0){
        requestAnimationFrame(animateDiffusion);
    }
}
