

var time_o2 = 0;

//$(document).ready(function(){
function graph( name,name_ch,unit,upp,low,upp_war,low_war)
{
	
				var data = [];
				var chart = new G2.Chart({
					container: name,
					forceFit: true,
					height: window.innerHeight / 2
					//	  height: window.innerHeight/3.5
				});
				chart.source(data, {
					time: {
						alias: '时间',
						type: 'time',
						mask: 'HH:mm:ss',
						tickInterval: 5000,
						nice: false
					},
					ver: {
						alias: unit,
						min: low,
						max: upp
					},
					type: {
						type: 'cat'
					}
				});
				chart.line().position('time*ver').color('type', ['#F5222D', '#cccccc']).shape('line').size(2).animate({
					update: {
						duration: 0
					}
				});

				chart.guide().line({
					top: true,
					start: ['min', upp_war],
					end: ['max', upp_war],

					lineStyle: {
						stroke: '#F5222D',
						lineWidth: 2
					},
					text: {
						content: '预警上限',
						position: 'start',
						offsetX: 20,
						offsetY: -5,
						style: {
							fontSize: 14,
							fill: '#F5222D',
							opacity: 0.5
						}
					}
				});

				chart.guide().line({
					top: true,
					start: ['min', low_war],
					end: ['max', low_war],

					lineStyle: {
						stroke: '#F5222D',
						lineWidth: 2
					},
					text: {
						content: '预警下限',
						position: 'start',
						offsetX: 20,
						offsetY: -5,
						style: {
							fontSize: 14,
							fill: '#F5222D',
							opacity: 0.5
						}
					}
				});
				chart.guide().regionFilter({
					top: true,
					start: ['min', low_war],
					end: ['max', upp_war],
					color: '#2593fc',
					apply: ['line']
				});
				chart.render();
				//		    var num = 0;
				setInterval(function () {
					var now = new Date();
					var time = now.getTime();
					//				var Blood_oxygen = Math.random() * 5 + 95;
					//				change("o2");
					var ver = parseFloat(document.getElementById("data_"+name).innerHTML);
					if (data.length >= 30) {
						data.shift();
					}
					data.push({
						time: time,
						ver: ver,
						type: name_ch
					});

					chart.changeData(data);
				}, 1000);

}


function myFunction_o2(flag) {
	var name = "o2";
	var name_ch = "血氧";
	var unit = '百分比(%)';
	var upp = 102;
	var low = 90;
	var upp_war = localStorage.getItem('o2_up');
	var low_war = localStorage.getItem('o2_low');
	if (flag === true) {
		if (time_o2 === 0) {
			time_o2 += 1;
			 graph( name,name_ch,unit,upp,low,upp_war,low_war);
		}

		//		document.write("hello1");
		$("#initShow_"+name).css("display", "none");
		$("#initHidden_"+name).css("display", "block");
		$("#"+name).css("display", "block");

	} else {
		$("#initShow_"+name).css("display", "block");
		$("#initHidden_"+name).css("display", "none");
		$("#"+name).css("display", "none");
	}
}

var time_bre = 0;
function myFunction_bre(flag) {
	var name = "bre";
	var name_ch = "心率";
	var unit = '次/分钟';
	var upp = 110;
	var low = 40;
	var upp_war = localStorage.getItem('hr_up');
	var low_war = localStorage.getItem('hr_low');
	if (flag === true) {
		if (time_bre === 0) {
			time_bre += 1;
			graph( name,name_ch,unit,upp,low,upp_war,low_war);
		}

		//		document.write("hello1");
		$("#initShow_"+name).css("display", "none");
		$("#initHidden_"+name).css("display", "block");
		$("#"+name).css("display", "block");

	} else {
		$("#initShow_"+name).css("display", "block");
		$("#initHidden_"+name).css("display", "none");
		$("#"+name).css("display", "none");
	}
}

var time_co2 = 0;
function myFunction_co2(flag) {
	
	var name = "co2";
	var name_ch = "二氧化碳";
	var unit = 'ppm';
	var upp = 1000;
	var low = 0;
	var upp_war = 700;
	var low_war = 300;
	if (flag === true) {
		if (time_co2 === 0)
		{
			time_co2 += 1;
			graph( name,name_ch,unit,upp,low,upp_war,low_war);
		}
		//		document.write("hello1");
		$("#initShow_"+name).css("display", "none");
		$("#initHidden_"+name).css("display", "block");
		$("#"+name).css("display", "block");

	} else {
		$("#initShow_"+name).css("display", "block");
		$("#initHidden_"+name).css("display", "none");
		$("#"+name).css("display", "none");
	}
}

var time_alc = 0;
function myFunction_alc(flag) {
	
	var name = "alc";
	var name_ch = "酒精浓度";
	var unit = 'ppm';
	var upp = 3000;
	var low = 0;
	var upp_war = 2200;
	var low_war = 1;
	if (flag === true) {
		if (time_alc === 0)
		{
			time_alc += 1;
			graph( name,name_ch,unit,upp,low,upp_war,low_war);
		}
		//		document.write("hello1");
		$("#initShow_"+name).css("display", "none");
		$("#initHidden_"+name).css("display", "block");
		$("#"+name).css("display", "block");

	} else {
		$("#initShow_"+name).css("display", "block");
		$("#initHidden_"+name).css("display", "none");
		$("#"+name).css("display", "none");
	}
}

var time_tem = 0;
function myFunction_tem(flag) {
	
	var name = "tem";
	var name_ch = "温度";
	var unit = '℃';
	var upp = 40;
	var low = 35;
	var upp_war = localStorage.getItem('temp_up');;
	var low_war = localStorage.getItem('temp_low');;
	if (flag === true) {
		if (time_tem === 0)
		{
			time_tem += 1;
			graph( name,name_ch,unit,upp,low,upp_war,low_war);
		}
		//		document.write("hello1");
		$("#initShow_"+name).css("display", "none");
		$("#initHidden_"+name).css("display", "block");
		$("#"+name).css("display", "block");

	} else {
		$("#initShow_"+name).css("display", "block");
		$("#initHidden_"+name).css("display", "none");
		$("#"+name).css("display", "none");
	}
}

var time_bp = 0;
function myFunction_bp(flag) {
	
	var name = "bp";
	var name_ch = "大气压";
	var unit = '分钟/次';
	var upp = 25;
	var low = 0;
	var upp_war = 20;
	var low_war = 12;
	if (flag === true) {
		if (time_bp === 0)
		{
			time_bp += 1;
			graph( name,name_ch,unit,upp,low,upp_war,low_war);
		}
		//		document.write("hello1");
		$("#initShow_"+name).css("display", "none");
		$("#initHidden_"+name).css("display", "block");
		$("#"+name).css("display", "block");

	} else {
		$("#initShow_"+name).css("display", "block");
		$("#initHidden_"+name).css("display", "none");
		$("#"+name).css("display", "none");
	}
}


//
//
//
//function mygraph()
//{
//	myFunction_o2()；
//	myFunction_bre()；
//	myFunction_co2()
//}