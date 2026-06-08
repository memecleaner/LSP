// ===========================
// THINGSPEAK CONFIG
// ===========================

const channelID = "3403065";
const readAPIKey = "V5UUGGEFV8OWCSG4";

// GANTI DENGAN IP ESP32
const espIP = "http://192.168.1.100";

// ===========================
// INISIALISASI CHART
// ===========================

const ctx =
document.getElementById("dhtChart")
.getContext("2d");

const dhtChart = new Chart(ctx,{

    type:'line',

    data:{

        labels:[],

        datasets:[

            {
                label:'Suhu (°C)',
                data:[],
                borderColor:'#e74c3c',
                backgroundColor:'#e74c3c',
                tension:0.4,
                fill:false
            },

            {
                label:'Kelembaban (%)',
                data:[],
                borderColor:'#3498db',
                backgroundColor:'#3498db',
                tension:0.4,
                fill:false
            }

        ]
    },

    options:{

        responsive:true,
        maintainAspectRatio:false,

        scales:{
            y:{
                beginAtZero:true
            }
        }
    }
});

// ===========================
// GAUGE INIT
// ===========================

function initGauge(){

    const progress =
    document.getElementById(
        "gauge-progress"
    );

    const length =
    progress.getTotalLength();

    progress.style.strokeDasharray =
    length;

    progress.style.strokeDashoffset =
    length;
}

// ===========================
// UPDATE GAUGE
// ===========================

function updateGauge(value){

    value =
    Math.max(
        0,
        Math.min(100,value)
    );

    const progress =
    document.getElementById(
        "gauge-progress"
    );

    const length =
    progress.getTotalLength();

    progress.style.strokeDasharray =
    length;

    progress.style.strokeDashoffset =
    length - (length * value / 100);

    document.getElementById(
        "soil-text"
    ).innerText =
    Math.round(value) + "%";
}

// ===========================
// AMBIL DATA THINGSPEAK
// ===========================

async function fetchThingSpeakData(){

    const url =
    `https://api.thingspeak.com/channels/${channelID}/feeds.json?api_key=${readAPIKey}&results=10`;

    try{

        const response =
        await fetch(url);

        const data =
        await response.json();

        document.getElementById(
            "status"
        ).innerText =
        "Terhubung ke Cloud";

        document.getElementById(
            "status"
        ).className =
        "online";

        dhtChart.data.labels = [];

        dhtChart.data.datasets[0].data = [];
        dhtChart.data.datasets[1].data = [];

        data.feeds.forEach(feed=>{

            const waktu =
            new Date(
                feed.created_at
            ).toLocaleTimeString();

            const suhu =
            parseFloat(
                feed.field1
            );

            const hum =
            parseFloat(
                feed.field2
            );

            dhtChart.data.labels.push(
                waktu
            );

            dhtChart.data.datasets[0]
            .data.push(
                isNaN(suhu) ? null : suhu
            );

            dhtChart.data.datasets[1]
            .data.push(
                isNaN(hum) ? null : hum
            );

        });

        dhtChart.update();

        const lastFeed =
        data.feeds[
            data.feeds.length - 1
        ];

        const soil =
        parseFloat(
            lastFeed.field3
        );

        updateGauge(
            isNaN(soil) ? 0 : soil
        );

    }
    catch(error){

        console.error(error);

        document.getElementById(
            "status"
        ).innerText =
        "Error Cloud";

        document.getElementById(
            "status"
        ).className =
        "offline";
    }
}

// ===========================
// RELAY CONTROL
// ===========================

function toggleRelay(relayNumber){

    const btn =
    document.getElementById(
        `btn-relay${relayNumber}`
    );

    const isOff =
    btn.classList.contains(
        "btn-off"
    );

    btn.className =
    isOff ?
    "btn btn-on" :
    "btn btn-off";

    const label =
    relayNumber === 1
    ? "Pompa Air"
    : "Solenoid";

    btn.innerText =
    `${label} : ${isOff ? 'ON':'OFF'}`;

    fetch(
        `${espIP}/relay?pin=${relayNumber}&state=${isOff ? 1 : 0}`
    )
    .catch(()=>{
        console.log(
            "ESP32 tidak merespon"
        );
    });
}

// ===========================
// START
// ===========================

window.onload = ()=>{

    initGauge();

    fetchThingSpeakData();

    setInterval(
        fetchThingSpeakData,
        15000
    );
};