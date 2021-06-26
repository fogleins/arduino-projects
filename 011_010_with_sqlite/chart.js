 function loadData() {
    $.ajax({
        method: "GET",
        url: "chart_data.php",
        timeout: 3000,
        dataType: "json",
        data: {},
        success: function(response) {
            if (response.success) {
                timestamps = [];
                values = [];
                for (i = 0; i < response.data.length; i++) {
                    timestamps.push(response.data[i].timestamp);
                    values.push(response.data[i].temperature);
                }
                chart = document.getElementById('chart');
                Plotly.newPlot(chart, [{
                    x: timestamps,
                    y: values }], { margin: { t: 0 } }
                );
                console.log("success");
            }
        }
    });
 }

 $(() => loadData());
