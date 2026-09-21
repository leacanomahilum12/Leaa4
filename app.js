// =====================================================
// LEAA ACTIVITY 4
// DHT11 FIREBASE MONITOR
// =====================================================

import {
    initializeApp
} from "https://www.gstatic.com/firebasejs/12.7.0/firebase-app.js";

import {
    getDatabase,
    ref,
    onValue
} from "https://www.gstatic.com/firebasejs/12.7.0/firebase-database.js";


// =====================================================
// FIREBASE CONFIGURATION
// =====================================================

const firebaseConfig = {

    apiKey:
        "AIzaSyDwL0Xf3YArAz_bW1ntZV7xWzgsxxyO82g",

    authDomain:
        "leaa-16562.firebaseapp.com",

    databaseURL:
        "https://leaa-16562-default-rtdb.europe-west1.firebasedatabase.app/",

    projectId:
        "leaa-16562",

    storageBucket:
        "lleaa-16562.firebasestorage.app",

    messagingSenderId:
        "364951684145",

    appId:
        "1:364951684145:web:d050f211166c5db9c00110",

    measurementId:
        "G-6BDDQNN76X"
};


// =====================================================
// INITIALIZE FIREBASE
// =====================================================

const firebaseApp =
    initializeApp(firebaseConfig);


// =====================================================
// DATABASE
// =====================================================

const database =
    getDatabase(firebaseApp);


// =====================================================
// DATABASE PATH
// =====================================================

const dataRef =
    ref(database, "ESP32_Data");


// =====================================================
// GLOBAL VARIABLES
// =====================================================

let allSensorData = {};

let selectedDate = "";

let sensorChart = null;


// =====================================================
// DOM ELEMENTS
// =====================================================

const statusElement =
    document.getElementById("firebaseStatus");

const currentTemperature =
    document.getElementById("currentTemperature");

const currentHumidity =
    document.getElementById("currentHumidity");

const graphDate =
    document.getElementById("graphDate");

const historyDate =
    document.getElementById("historyDate");

const historyBody =
    document.getElementById("historyTableBody");

const recordCount =
    document.getElementById("recordCount");

const toggleHistory =
    document.getElementById("toggleHistory");

const historyContent =
    document.getElementById("historyContent");


// =====================================================
// STATUS
// =====================================================

function setStatus(text, connected) {

    if (!statusElement) {
        return;
    }

    statusElement.textContent =
        "Firebase Status: " + text;

    if (connected) {

        statusElement.classList.remove(
            "status-error"
        );

        statusElement.classList.add(
            "status-connected"
        );

    } else {

        statusElement.classList.remove(
            "status-connected"
        );

        statusElement.classList.add(
            "status-error"
        );
    }
}


// =====================================================
// NUMBER HELPER
// =====================================================

function toNumber(value) {

    const number = Number(value);

    if (Number.isFinite(number)) {
        return number;
    }

    return null;
}


// =====================================================
// FORMAT NUMBER
// =====================================================

function formatNumber(value) {

    if (
        value === null ||
        value === undefined
    ) {
        return "--";
    }

    return Number(value).toFixed(1);
}


// =====================================================
// GET LATEST READING
// =====================================================

function getLatestReading(data) {

    let latest = null;

    const dates =
        Object.keys(data || {}).sort();

    for (const date of dates) {

        const times =
            Object.keys(
                data[date] || {}
            ).sort();

        for (const time of times) {

            const reading =
                data[date][time];

            if (
                !reading ||
                typeof reading !== "object"
            ) {
                continue;
            }

            const temperature =
                toNumber(
                    reading.temperature
                );

            const humidity =
                toNumber(
                    reading.humidity
                );

            if (
                temperature === null &&
                humidity === null
            ) {
                continue;
            }

            latest = {

                date:
                    date,

                time:
                    time,

                temperature:
                    temperature,

                humidity:
                    humidity
            };
        }
    }

    return latest;
}


// =====================================================
// DATE LIST
// =====================================================

function getDateList(data) {

    return Object.keys(data || {})
        .filter(
            key =>
                data[key] &&
                typeof data[key] === "object"
        )
        .sort()
        .reverse();
}


// =====================================================
// POPULATE DATE SELECTS
// =====================================================

function populateDateSelect() {

    const dates =
        getDateList(allSensorData);


    // =================================================
    // GRAPH DATE
    // =================================================

    if (graphDate) {

        graphDate.innerHTML = "";

        if (dates.length === 0) {

            const option =
                document.createElement("option");

            option.value = "";

            option.textContent =
                "No dates available";

            graphDate.appendChild(option);

        } else {

            dates.forEach(date => {

                const option =
                    document.createElement("option");

                option.value = date;

                option.textContent = date;

                graphDate.appendChild(option);
            });

            graphDate.value =
                selectedDate || dates[0];
        }
    }


    // =================================================
    // HISTORY DATE
    // =================================================

    if (historyDate) {

        historyDate.innerHTML = "";

        if (dates.length === 0) {

            const option =
                document.createElement("option");

            option.value = "";

            option.textContent =
                "No dates available";

            historyDate.appendChild(option);

        } else {

            dates.forEach(date => {

                const option =
                    document.createElement("option");

                option.value = date;

                option.textContent = date;

                historyDate.appendChild(option);
            });

            historyDate.value =
                selectedDate || dates[0];
        }
    }
}


// =====================================================
// CURRENT READING
// =====================================================

function updateCurrentReading() {

    const latest =
        getLatestReading(allSensorData);

    if (!latest) {

        if (currentTemperature) {
            currentTemperature.textContent =
                "-- °C";
        }

        if (currentHumidity) {
            currentHumidity.textContent =
                "-- %";
        }

        return;
    }

    if (currentTemperature) {

        currentTemperature.textContent =
            formatNumber(
                latest.temperature
            ) + " °C";
    }

    if (currentHumidity) {

        currentHumidity.textContent =
            formatNumber(
                latest.humidity
            ) + " %";
    }

    console.log(
        "LATEST READING:",
        latest
    );
}


// =====================================================
// READINGS FOR DATE
// =====================================================

function getReadingsForDate(date) {

    const result = [];

    if (!date) {
        return result;
    }

    const dayData =
        allSensorData[date];

    if (
        !dayData ||
        typeof dayData !== "object"
    ) {
        return result;
    }

    const times =
        Object.keys(dayData)
            .filter(
                time =>
                    dayData[time] &&
                    typeof dayData[time] === "object"
            )
            .sort();

    times.forEach(time => {

        const reading =
            dayData[time];

        const temperature =
            toNumber(
                reading.temperature
            );

        const humidity =
            toNumber(
                reading.humidity
            );

        if (
            temperature === null &&
            humidity === null
        ) {
            return;
        }

        result.push({

            time:
                time,

            temperature:
                temperature,

            humidity:
                humidity
        });
    });

    return result;
}


// =====================================================
// UPDATE CHART
// =====================================================

function updateChart() {

    if (
        typeof Chart === "undefined"
    ) {

        console.error(
            "Chart.js is not loaded."
        );

        return;
    }

    const date =
        graphDate
            ? graphDate.value
            : selectedDate;

    const readings =
        getReadingsForDate(date);

    const labels =
        readings.map(
            item => item.time
        );

    const temperatures =
        readings.map(
            item => item.temperature
        );

    const humidities =
        readings.map(
            item => item.humidity
        );

    const canvas =
        document.getElementById(
            "sensorChart"
        );

    if (!canvas) {
        return;
    }

    if (sensorChart) {

        sensorChart.destroy();

        sensorChart = null;
    }

    sensorChart =
        new Chart(
            canvas,
            {

                type:
                    "line",

                data:
                {
                    labels:
                        labels,

                    datasets:
                    [

                        {
                            label:
                                "Temperature (°C)",

                            data:
                                temperatures,

                            yAxisID:
                                "temperature",

                            tension:
                                0.3,

                            borderWidth:
                                3,

                            pointRadius:
                                4,

                            borderColor:
                                "#5ee7ff",

                            backgroundColor:
                                "rgba(94,231,255,0.12)"
                        },

                        {
                            label:
                                "Humidity (%)",

                            data:
                                humidities,

                            yAxisID:
                                "humidity",

                            tension:
                                0.3,

                            borderWidth:
                                3,

                            pointRadius:
                                4,

                            borderColor:
                                "#7774ff",

                            backgroundColor:
                                "rgba(119,116,255,0.12)"
                        }

                    ]
                },

                options:
                {

                    responsive:
                        true,

                    maintainAspectRatio:
                        false,

                    animation:
                    {
                        duration:
                            300
                    },

                    interaction:
                    {
                        mode:
                            "index",

                        intersect:
                            false
                    },

                    plugins:
                    {
                        legend:
                        {
                            labels:
                            {
                                color:
                                    "#cbd7e7"
                            }
                        }
                    },

                    scales:
                    {

                        x:
                        {
                            ticks:
                            {
                                color:
                                    "#8192ad"
                            },

                            grid:
                            {
                                color:
                                    "#202f46"
                            }
                        },

                        temperature:
                        {
                            type:
                                "linear",

                            position:
                                "left",

                            ticks:
                            {
                                color:
                                    "#5ee7ff"
                            },

                            title:
                            {
                                display:
                                    true,

                                text:
                                    "Temperature (°C)",

                                color:
                                    "#5ee7ff"
                            },

                            grid:
                            {
                                color:
                                    "#202f46"
                            }
                        },

                        humidity:
                        {
                            type:
                                "linear",

                            position:
                                "right",

                            ticks:
                            {
                                color:
                                    "#7774ff"
                            },

                            title:
                            {
                                display:
                                    true,

                                text:
                                    "Humidity (%)",

                                color:
                                    "#7774ff"
                            },

                            grid:
                            {
                                drawOnChartArea:
                                    false
                            }
                        }
                    }
                }
            }
        );

    console.log(
        "GRAPH UPDATED:",
        date,
        readings.length,
        "records"
    );
}


// =====================================================
// UPDATE HISTORY
// =====================================================

function updateHistory() {

    if (!historyBody) {
        return;
    }

    const date =
        historyDate
            ? historyDate.value
            : selectedDate;

    const readings =
        getReadingsForDate(date);

    historyBody.innerHTML =
        "";

    if (
        readings.length === 0
    ) {

        const row =
            document.createElement("tr");

        const cell =
            document.createElement("td");

        cell.colSpan = 3;

        cell.textContent =
            "No sensor data available.";

        row.appendChild(cell);

        historyBody.appendChild(row);

        if (recordCount) {
            recordCount.textContent =
                "0 records";
        }

        return;
    }


    // =================================================
    // LATEST FIRST
    // =================================================

    readings
        .slice()
        .reverse()
        .forEach(reading => {

            const row =
                document.createElement("tr");

            const timeCell =
                document.createElement("td");

            const temperatureCell =
                document.createElement("td");

            const humidityCell =
                document.createElement("td");


            timeCell.textContent =
                reading.time;

            temperatureCell.textContent =
                formatNumber(
                    reading.temperature
                ) + " °C";

            humidityCell.textContent =
                formatNumber(
                    reading.humidity
                ) + " %";


            row.appendChild(timeCell);

            row.appendChild(
                temperatureCell
            );

            row.appendChild(
                humidityCell
            );

            historyBody.appendChild(row);
        });


    if (recordCount) {

        recordCount.textContent =
            readings.length +
            (
                readings.length === 1
                    ? " record"
                    : " records"
            );
    }
}


// =====================================================
// DASHBOARD
// =====================================================

function updateDashboard() {

    const dates =
        getDateList(allSensorData);

    console.log(
        "AVAILABLE DATES:",
        dates
    );


    if (
        dates.length === 0
    ) {

        if (currentTemperature) {

            currentTemperature.textContent =
                "-- °C";
        }

        if (currentHumidity) {

            currentHumidity.textContent =
                "-- %";
        }

        if (historyBody) {

            historyBody.innerHTML = `
                <tr>
                    <td colspan="3">
                        No sensor data available.
                    </td>
                </tr>
            `;
        }

        if (recordCount) {

            recordCount.textContent =
                "0 records";
        }

        return;
    }


    if (
        !selectedDate ||
        !dates.includes(selectedDate)
    ) {

        selectedDate =
            dates[0];
    }


    populateDateSelect();

    updateCurrentReading();

    updateChart();

    updateHistory();
}


// =====================================================
// FIREBASE LISTENER
// =====================================================

console.log(
    "================================="
);

console.log(
    "LEAA ACTIVITY 4"
);

console.log(
    "CONNECTING TO FIREBASE"
);

console.log(
    "Firebase Project: leaa-abc04"
);

console.log(
    "Database: /ESP32_Data"
);

console.log(
    "================================="
);


setStatus(
    "Connecting...",
    false
);


onValue(

    dataRef,

    snapshot => {

        console.log(
            "FIREBASE DATA RECEIVED"
        );

        const value =
            snapshot.val();

        console.log(
            "FIREBASE VALUE:",
            value
        );

        allSensorData =
            value || {};

        setStatus(
            "Connected",
            true
        );

        updateDashboard();
    },

    error => {

        console.error(
            "FIREBASE READ ERROR:",
            error
        );

        setStatus(
            "Error",
            false
        );
    }

);


// =====================================================
// GRAPH DATE CHANGE
// =====================================================

if (graphDate) {

    graphDate.addEventListener(
        "change",
        function () {

            selectedDate =
                this.value;

            if (historyDate) {
                historyDate.value =
                    selectedDate;
            }

            updateChart();

            updateHistory();
        }
    );
}


// =====================================================
// HISTORY DATE CHANGE
// =====================================================

if (historyDate) {

    historyDate.addEventListener(
        "change",
        function () {

            selectedDate =
                this.value;

            if (graphDate) {
                graphDate.value =
                    selectedDate;
            }

            updateHistory();

            updateChart();
        }
    );
}


// =====================================================
// SHOW / HIDE HISTORY
// =====================================================

if (
    toggleHistory &&
    historyContent
) {

    toggleHistory.addEventListener(
        "click",
        function () {

            if (
                historyContent.classList.contains(
                    "hidden"
                )
            ) {

                historyContent.classList.remove(
                    "hidden"
                );

                toggleHistory.textContent =
                    "Hide History";

                updateHistory();

            } else {

                historyContent.classList.add(
                    "hidden"
                );

                toggleHistory.textContent =
                    "Show History";
            }
        }
    );
}


// =====================================================
// INITIAL STATUS
// =====================================================

setStatus(
    "Connecting...",
    false
);