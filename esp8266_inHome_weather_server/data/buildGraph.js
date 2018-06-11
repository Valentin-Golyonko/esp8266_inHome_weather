var dataArrayTempr = [];
var dataArrayHum = [];
var dataArrayAir = [];
var dataArrayPre = [];

var defaultZoomTime = 24*60*60*1000; // 1 day
var minZoom = -6; // 22 minutes 30 seconds
var maxZoom = 8; // ~ 8.4 months

var zoomLevel = 0;
var viewportEndTime = new Date();
var viewportStartTime = new Date();

var chartNumber = 1; // 1 - tempr, 2 - hum, 3 - air, 4 - pressure

loadCSV(); // Download the CSV data, load Google Charts, parse the data, and draw the chart

/*
Structure:

    loadCSV
        callback:
        parseCSV
        load Google Charts (anonymous)
            callback:
            updateViewport
                displayDate
                drawChart
*/

/*
               |                    CHART                    |
               |                  VIEW PORT                  |
invisible      |                   visible                   |      invisible
---------------|---------------------------------------------|--------------->  time
       viewportStartTime                              viewportEndTime

               |______________viewportWidthTime______________|

viewportWidthTime = 1 day * 2^zoomLevel = viewportEndTime - viewportStartTime
*/

function loadCSV() {
	if(chartNumber == 1) {
		loadCSVTempr();
	} else if(chartNumber == 2) {
		loadCSVHum();
	} else if(chartNumber == 3) {
		loadCSVAir();
	} else if(chartNumber == 4) {
		loadCSVPre();
	}
}

function loadCSVTempr() {
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            dataArrayTempr = parseCSVTempr(this.responseText);
            google.charts.load('current', { 'packages': ['line', 'corechart'] });
            google.charts.setOnLoadCallback(updateViewport);
        }
    };
    xmlhttp.open("GET", "tempr.csv", true);
    xmlhttp.send();
    var loadingdiv = document.getElementById("loading");
    loadingdiv.style.visibility = "visible";
}

function loadCSVHum() {
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            dataArrayHum = parseCSVHum(this.responseText);
            google.charts.load('current', { 'packages': ['line', 'corechart'] });
            google.charts.setOnLoadCallback(updateViewport);
        }
    };
    xmlhttp.open("GET", "hum.csv", true);
    xmlhttp.send();
    var loadingdiv = document.getElementById("loading");
    loadingdiv.style.visibility = "visible";
}

function loadCSVAir() {
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            dataArrayAir = parseCSVAir(this.responseText);
            google.charts.load('current', { 'packages': ['line', 'corechart'] });
            google.charts.setOnLoadCallback(updateViewport);
        }
    };
    xmlhttp.open("GET", "air.csv", true);
    xmlhttp.send();
    var loadingdiv = document.getElementById("loading");
    loadingdiv.style.visibility = "visible";
}

function loadCSVPre() {
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            dataArrayPre = parseCSVPre(this.responseText);
            google.charts.load('current', { 'packages': ['line', 'corechart'] });
            google.charts.setOnLoadCallback(updateViewport);
        }
    };
    xmlhttp.open("GET", "pre.csv", true);
    xmlhttp.send();
    var loadingdiv = document.getElementById("loading");
    loadingdiv.style.visibility = "visible";
}

function parseCSVTempr(string) {
    var array = [];
    var lines = string.split("\n");
    for (var i = 0; i < lines.length; i++) {
        var data = lines[i].split(",", 2);
        data[0] = new Date(parseInt(data[0]) * 1000);
        data[1] = parseFloat(data[1]);
        array.push(data);
    }
    return array;
}

function parseCSVHum(string) {
    var array = [];
    var lines = string.split("\n");
    for (var i = 0; i < lines.length; i++) {
        var data = lines[i].split(",", 2);
        data[0] = new Date(parseInt(data[0]) * 1000);
        data[1] = parseFloat(data[1]);
        array.push(data);
    }
    return array;
}

function parseCSVAir(string) {
    var array = [];
    var lines = string.split("\n");
    for (var i = 0; i < lines.length; i++) {
        var data = lines[i].split(",", 2);
        data[0] = new Date(parseInt(data[0]) * 1000);
        data[1] = parseFloat(data[1]);
        array.push(data);
    }
    return array;
}

function parseCSVPre(string) {
    var array = [];
    var lines = string.split("\n");
    for (var i = 0; i < lines.length; i++) {
        var data = lines[i].split(",", 2);
        data[0] = new Date(parseInt(data[0]) * 1000);
        data[1] = parseFloat(data[1]);
        array.push(data);
    }
    return array;
}

function drawChartTempr() {
    var data = new google.visualization.DataTable();
    data.addColumn('datetime', 'UNIX');
    data.addColumn('number', 'Temperature');
    data.addRows(dataArrayTempr);

    var options = {
        curveType: 'function',
        height: 380,
        legend: { position: 'none' },
        hAxis: {
            viewWindow: {
                min: viewportStartTime,
                max: viewportEndTime
            },
            gridlines: {
                count: -1,
                units: {
                    days: { format: ['MMM dd'] },
                    hours: { format: ['HH:mm', 'ha'] },
                }
            },
            minorGridlines: {
                units: {
                    hours: { format: ['hh:mm:ss a', 'ha'] },
                    minutes: { format: ['HH:mm a Z', ':mm'] }
                }
            }
        },
        vAxis: {
            title: "Temperature (Celsius)"
        }
    };

    var chart = new google.visualization.LineChart(document.getElementById('chart_div'));
    chart.draw(data, options);
	
    var loadingdiv = document.getElementById("loading");
    loadingdiv.style.visibility = "hidden";
}

function drawChartHum() {
    var data = new google.visualization.DataTable();
    data.addColumn('datetime', 'UNIX');
    data.addColumn('number', 'Humidity');
    data.addRows(dataArrayHum);

    var options = {
        curveType: 'function',
        height: 380,
        legend: { position: 'none' },
        hAxis: {
            viewWindow: {
                min: viewportStartTime,
                max: viewportEndTime
            },
            gridlines: {
                count: -1,
                units: {
                    days: { format: ['MMM dd'] },
                    hours: { format: ['HH:mm', 'ha'] },
                }
            },
            minorGridlines: {
                units: {
                    hours: { format: ['hh:mm:ss a', 'ha'] },
                    minutes: { format: ['HH:mm a Z', ':mm'] }
                }
            }
        },
        vAxis: {
            title: "Humidity (%)"
        }
    };

    var chart = new google.visualization.LineChart(document.getElementById('chart_div'));
    chart.draw(data, options);
	
    var loadingdiv = document.getElementById("loading");
    loadingdiv.style.visibility = "hidden";
}

function drawChartAir() {
    var data = new google.visualization.DataTable();
    data.addColumn('datetime', 'UNIX');
    data.addColumn('number', 'Air');
    data.addRows(dataArrayAir);

    var options = {
        curveType: 'function',
        height: 380,
        legend: { position: 'none' },
        hAxis: {
            viewWindow: {
                min: viewportStartTime,
                max: viewportEndTime
            },
            gridlines: {
                count: -1,
                units: {
                    days: { format: ['MMM dd'] },
                    hours: { format: ['HH:mm', 'ha'] },
                }
            },
            minorGridlines: {
                units: {
                    hours: { format: ['hh:mm:ss a', 'ha'] },
                    minutes: { format: ['HH:mm a Z', ':mm'] }
                }
            }
        },
        vAxis: {
            title: "Air quality (ppm)"
        }
    };

    var chart = new google.visualization.LineChart(document.getElementById('chart_div'));
    chart.draw(data, options);
	
    var loadingdiv = document.getElementById("loading");
    loadingdiv.style.visibility = "hidden";
}

function drawChartPre() {
    var data = new google.visualization.DataTable();
    data.addColumn('datetime', 'UNIX');
    data.addColumn('number', 'Pressure');
    data.addRows(dataArrayPre);

    var options = {
        curveType: 'function',
        height: 380,
        legend: { position: 'none' },
        hAxis: {
            viewWindow: {
                min: viewportStartTime,
                max: viewportEndTime
            },
            gridlines: {
                count: -1,
                units: {
                    days: { format: ['MMM dd'] },
                    hours: { format: ['HH:mm', 'ha'] },
                }
            },
            minorGridlines: {
                units: {
                    hours: { format: ['hh:mm:ss a', 'ha'] },
                    minutes: { format: ['HH:mm a Z', ':mm'] }
                }
            }
        },
        vAxis: {
            title: "Pressure (mmHg)"
        }
    };

    var chart = new google.visualization.LineChart(document.getElementById('chart_div'));
    chart.draw(data, options);
	
    var loadingdiv = document.getElementById("loading");
    loadingdiv.style.visibility = "hidden";
}

function displayDate() { // Display the start and end date on the page
    var dateDiv = document.getElementById("date");
    var endDay = viewportEndTime.getDate();
    var endMonth = viewportEndTime.getMonth();
    var startDay = viewportStartTime.getDate();
    var startMonth = viewportStartTime.getMonth()
    if (endDay == startDay && endMonth == startMonth) {
        dateDiv.textContent = (endDay).toString() + "/" + (endMonth + 1).toString();
    } else {
        dateDiv.textContent = (startDay).toString() + "/" + (startMonth + 1).toString() + " - " + (endDay).toString() + "/" + (endMonth + 1).toString();
    }
}

document.getElementById("prev").onclick = function() {
    viewportEndTime = new Date(viewportEndTime.getTime() - getViewportWidthTime()/3); // move the viewport to the left for one third of its width (e.g. if the viewport width is 3 days, move one day back in time)
    updateViewport();
}
document.getElementById("next").onclick = function() {
    viewportEndTime = new Date(viewportEndTime.getTime() + getViewportWidthTime()/3); // move the viewport to the right for one third of its width (e.g. if the viewport width is 3 days, move one day into the future)
    updateViewport();
}

document.getElementById("zoomout").onclick = function() {
    zoomLevel += 1; // increment the zoom level (zoom out)
    if(zoomLevel > maxZoom) zoomLevel = maxZoom;
    else updateViewport();
}
document.getElementById("zoomin").onclick = function() {
    zoomLevel -= 1; // decrement the zoom level (zoom in)
    if(zoomLevel < minZoom) zoomLevel = minZoom;
    else updateViewport();
}

document.getElementById("reset").onclick = function() {
    viewportEndTime = new Date(); // the end time of the viewport is the current time
    zoomLevel = 0; // reset the zoom level to the default (one day)
    updateViewport();
}
document.getElementById("refresh").onclick = function() {
    viewportEndTime = new Date(); // the end time of the viewport is the current time
	loadCSV();
}

document.getElementById("getTempr").onclick = function() {
    viewportEndTime = new Date(); // the end time of the viewport is the current time
    loadCSVTempr(); // download the latest data and re-draw the chart
	chartNumber = 1;
}

document.getElementById("getHum").onclick = function() {
    viewportEndTime = new Date(); // the end time of the viewport is the current time
    loadCSVHum(); // download the latest data and re-draw the chart
	chartNumber = 2;
}

document.getElementById("getAir").onclick = function() {
    viewportEndTime = new Date(); // the end time of the viewport is the current time
    loadCSVAir(); // download the latest data and re-draw the chart
	chartNumber = 3;
}

document.getElementById("getPre").onclick = function() {
    viewportEndTime = new Date(); // the end time of the viewport is the current time
    loadCSVPre(); // download the latest data and re-draw the chart
	chartNumber = 4;
}

document.body.onresize = function() {
	if(chartNumber == 1) {
		drawChartTempr;
	} else if(chartNumber == 2) {
		drawChartHum;
	} else if(chartNumber == 3) {
		drawChartAir;
	} else if(chartNumber == 4) {
		drawChartPre;
	}
}

function updateViewport() {
    viewportStartTime = new Date(viewportEndTime.getTime() - getViewportWidthTime());
	displayDate();
	if(chartNumber == 1) {
		drawChartTempr();
	} else if(chartNumber == 2) {
		drawChartHum();
	} else if(chartNumber == 3) {
		drawChartAir();
	} else if(chartNumber == 4) {
		drawChartPre();
	}
}
function getViewportWidthTime() {
    return defaultZoomTime*(2**zoomLevel); // exponential relation between zoom level and zoom time span
                                           // every time you zoom, you double or halve the time scale
}
