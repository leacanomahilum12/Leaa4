#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <FirebaseClient.h>

#include <LittleFS.h>

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#include <DHT.h>

#include <time.h>


// =====================================================
// DHT11
// =====================================================

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(
    DHTPIN,
    DHTTYPE
);


// =====================================================
// FIREBASE
// =====================================================

#define API_KEY \
"AIzaSyDwL0Xf3YArAz_bW1ntZV7xWzgsxxyO82g"

#define DATABASE_URL \
"https://leaa-16562-default-rtdb.europe-west1.firebasedatabase.app/"


// =====================================================
// FIREBASE AUTH
// =====================================================
//
// IMPORTANT:
// Replace these with the Firebase Authentication
// email and password used for your ESP32.
//
// Do NOT post the password here.
//

#define USER_EMAIL \
"leacanomahilum@gmail.com"

#define USER_PASSWORD \
"Lea12345.@"


// =====================================================
// FIREBASE OBJECTS
// =====================================================

UserAuth user_auth(
    API_KEY,
    USER_EMAIL,
    USER_PASSWORD,
    3000
);

FirebaseApp app;

WiFiClientSecure ssl_client;

AsyncClientClass aClient(
    ssl_client
);

RealtimeDatabase Database;


// =====================================================
// WEB SERVER
// =====================================================

AsyncWebServer server(
    80
);


// =====================================================
// WIFI MANAGER
// =====================================================

const char *AP_SSID =
    "ESP-WIFI-MANAGER";

bool wifiManagerMode =
    false;


// =====================================================
// SENSOR INTERVAL
// =====================================================
//
// Kim's original function:
// sensor reading every 10 seconds.
//

const unsigned long SENSOR_INTERVAL =
    10000;

unsigned long lastSensorRead =
    0;


// =====================================================
// FUNCTION DECLARATIONS
// =====================================================

void processFirebase(
    AsyncResult &aResult
);

bool connectToSavedWiFi();

void startWiFiManager();

void startMainWebServer();

void setupFirebase();

void sendSensorData();

String readFile(
    const char *path
);

bool writeFile(
    const char *path,
    const String &data
);

void deleteWiFiFiles();

String getDateString();

String getTimeString();


// =====================================================
// READ FILE
// =====================================================

String readFile(
    const char *path
)
{

    if (
        !LittleFS.exists(path)
    )
    {
        return "";
    }


    File file =
        LittleFS.open(
            path,
            "r"
        );


    if (!file)
    {
        return "";
    }


    String data =
        file.readString();


    file.close();


    data.trim();


    return data;
}


// =====================================================
// WRITE FILE
// =====================================================

bool writeFile(
    const char *path,
    const String &data
)
{

    File file =
        LittleFS.open(
            path,
            "w"
        );


    if (!file)
    {

        Serial.print(
            "Failed to open file: "
        );

        Serial.println(
            path
        );

        return false;
    }


    file.print(
        data
    );


    file.close();


    return true;
}


// =====================================================
// DELETE WIFI FILES
// =====================================================

void deleteWiFiFiles()
{

    LittleFS.remove(
        "/ssid.txt"
    );

    LittleFS.remove(
        "/pass.txt"
    );

    LittleFS.remove(
        "/ip.txt"
    );

    LittleFS.remove(
        "/gateway.txt"
    );


    Serial.println(
        "WiFi settings deleted."
    );
}


// =====================================================
// CONNECT SAVED WIFI
// =====================================================

bool connectToSavedWiFi()
{

    String ssid =
        readFile(
            "/ssid.txt"
        );

    String pass =
        readFile(
            "/pass.txt"
        );

    String ip =
        readFile(
            "/ip.txt"
        );

    String gateway =
        readFile(
            "/gateway.txt"
        );


    if (
        ssid.length() == 0
    )
    {

        Serial.println();

        Serial.println(
            "NO CONNECTED WIFI"
        );

        Serial.println(
            "No saved WiFi credentials."
        );

        return false;
    }


    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "        SAVED WIFI FOUND"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "SSID: "
    );

    Serial.println(
        ssid
    );


    WiFi.mode(
        WIFI_STA
    );


    delay(
        500
    );


    // =================================================
    // STATIC IP
    // =================================================

    if (
        ip.length() > 0 &&
        gateway.length() > 0
    )
    {

        IPAddress localIP;

        IPAddress gatewayIP;


        if (
            localIP.fromString(ip) &&
            gatewayIP.fromString(gateway)
        )
        {

            IPAddress subnet(
                255,
                255,
                255,
                0
            );


            if (
                WiFi.config(
                    localIP,
                    gatewayIP,
                    subnet
                )
            )
            {

                Serial.println(
                    "Static IP configured."
                );

            }
            else
            {

                Serial.println(
                    "Static IP configuration failed."
                );
            }
        }
    }


    // =================================================
    // WIFI BEGIN
    // =================================================

    WiFi.begin(
        ssid.c_str(),
        pass.c_str()
    );


    Serial.print(
        "Connecting to WiFi"
    );


    unsigned long startTime =
        millis();


    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 20000
    )
    {

        Serial.print(
            "."
        );

        delay(
            500
        );
    }


    Serial.println();


    // =================================================
    // SUCCESS
    // =================================================

    if (
        WiFi.status() ==
        WL_CONNECTED
    )
    {

        Serial.println();

        Serial.println(
            "================================="
        );

        Serial.println(
            "          WIFI CONNECTED"
        );

        Serial.println(
            "================================="
        );


        Serial.print(
            "SSID: "
        );

        Serial.println(
            WiFi.SSID()
        );


        Serial.print(
            "IP Address: "
        );

        Serial.println(
            WiFi.localIP()
        );


        Serial.print(
            "Gateway: "
        );

        Serial.println(
            WiFi.gatewayIP()
        );


        Serial.println();


        return true;
    }


    // =================================================
    // FAILED
    // =================================================

    Serial.println();

    Serial.println(
        "NO CONNECTED WIFI"
    );

    Serial.println(
        "Failed to connect to saved WiFi."
    );


    WiFi.disconnect(
        true
    );


    delay(
        1000
    );


    return false;
}


// =====================================================
// WIFI MANAGER
// =====================================================

void startWiFiManager()
{

    wifiManagerMode =
        true;


    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "        WIFI MANAGER MODE"
    );

    Serial.println(
        "================================="
    );


    WiFi.mode(
        WIFI_AP
    );


    delay(
        500
    );


    bool apStarted =
        WiFi.softAP(
            AP_SSID
        );


    if (!apStarted)
    {

        Serial.println(
            "ERROR: Failed to start WiFi Manager AP!"
        );
    }


    delay(
        1000
    );


    Serial.print(
        "AP SSID: "
    );

    Serial.println(
        AP_SSID
    );


    Serial.print(
        "AP IP Address: "
    );

    Serial.println(
        WiFi.softAPIP()
    );


    // =================================================
    // WIFI MANAGER PAGE
    // =================================================

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {

            if (
                LittleFS.exists(
                    "/wifimanager.html"
                )
            )
            {

                request->send(
                    LittleFS,
                    "/wifimanager.html",
                    "text/html"
                );

            }
            else
            {

                request->send(
                    404,
                    "text/plain",
                    "wifimanager.html not found."
                );
            }

        }
    );


    // =================================================
    // SAVE WIFI
    // =================================================

    server.on(
        "/",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {

            String ssid = "";

            String pass = "";

            String ip = "";

            String gateway = "";


            if (
                request->hasParam(
                    "ssid",
                    true
                )
            )
            {

                ssid =
                    request
                    ->getParam(
                        "ssid",
                        true
                    )
                    ->value();
            }


            if (
                request->hasParam(
                    "pass",
                    true
                )
            )
            {

                pass =
                    request
                    ->getParam(
                        "pass",
                        true
                    )
                    ->value();
            }


            if (
                request->hasParam(
                    "ip",
                    true
                )
            )
            {

                ip =
                    request
                    ->getParam(
                        "ip",
                        true
                    )
                    ->value();
            }


            if (
                request->hasParam(
                    "gateway",
                    true
                )
            )
            {

                gateway =
                    request
                    ->getParam(
                        "gateway",
                        true
                    )
                    ->value();
            }


            ssid.trim();

            pass.trim();

            ip.trim();

            gateway.trim();


            if (
                ssid.length() == 0 ||
                pass.length() == 0
            )
            {

                request->send(
                    400,
                    "text/plain",
                    "SSID and password are required."
                );

                return;
            }


            writeFile(
                "/ssid.txt",
                ssid
            );

            writeFile(
                "/pass.txt",
                pass
            );

            writeFile(
                "/ip.txt",
                ip
            );

            writeFile(
                "/gateway.txt",
                gateway
            );


            request->send(
                200,
                "text/html",

                "<html>"
                "<head>"
                "<meta name='viewport' "
                "content='width=device-width, initial-scale=1'>"
                "</head>"

                "<body style='font-family:Arial;"
                "text-align:center;"
                "padding:50px;"
                "background:#0b1220;"
                "color:white;'>"

                "<div style='background:#111d31;"
                "padding:30px;"
                "border-radius:20px;"
                "max-width:500px;"
                "margin:auto;"
                "border:1px solid #30476a;'>"

                "<h1 style='color:#5ee7ff;'>"
                "WiFi Saved!"
                "</h1>"

                "<p>"
                "The ESP32 will restart and "
                "connect to the saved WiFi."
                "</p>"

                "<p>"
                "Please wait..."
                "</p>"

                "</div>"

                "</body>"
                "</html>"
            );


            delay(
                1500
            );


            ESP.restart();
        }
    );


    server.begin();


    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "       WIFI MANAGER READY"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "Connect to WiFi: "
    );

    Serial.println(
        AP_SSID
    );


    Serial.print(
        "Then open: http://"
    );

    Serial.println(
        WiFi.softAPIP()
    );


    Serial.println();
}


// =====================================================
// MAIN WEB SERVER
// =====================================================

void startMainWebServer()
{

    wifiManagerMode =
        false;


    // =================================================
    // MAIN WEBSITE
    // =================================================

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {

            if (
                LittleFS.exists(
                    "/index.html"
                )
            )
            {

                request->send(
                    LittleFS,
                    "/index.html",
                    "text/html"
                );

            }
            else
            {

                request->send(
                    404,
                    "text/plain",
                    "index.html not found."
                );
            }

        }
    );


    // =================================================
    // CHANGE WIFI
    // =================================================

    server.on(
        "/change-wifi",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {

            request->send(
                200,
                "text/html",

                "<html>"
                "<head>"
                "<meta name='viewport' "
                "content='width=device-width, initial-scale=1'>"
                "</head>"

                "<body style='font-family:Arial;"
                "text-align:center;"
                "padding:50px;"
                "background:#0b1220;"
                "color:white;'>"

                "<div style='background:#111d31;"
                "padding:30px;"
                "border-radius:20px;"
                "max-width:500px;"
                "margin:auto;"
                "border:1px solid #30476a;'>"

                "<h1 style='color:#5ee7ff;'>"
                "Changing WiFi..."
                "</h1>"

                "<p>"
                "WiFi settings will be cleared."
                "</p>"

                "<p>"
                "The ESP32 will restart."
                "</p>"

                "</div>"

                "</body>"
                "</html>"
            );


            delay(
                1000
            );


            deleteWiFiFiles();


            ESP.restart();
        }
    );


    // =================================================
    // STATIC FILES
    // =================================================

    server.serveStatic(
        "/",
        LittleFS,
        "/"
    );


    server.begin();


    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "      MAIN WEB SERVER READY"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "Website: http://"
    );

    Serial.println(
        WiFi.localIP()
    );


    Serial.println();
}


// =====================================================
// FIREBASE CALLBACK
// =====================================================

void processFirebase(
    AsyncResult &aResult
)
{

    if (
        !aResult.isResult()
    )
    {
        return;
    }


    // =================================================
    // EVENT
    // =================================================

    if (
        aResult.isEvent()
    )
    {

        Firebase.printf(
            "Firebase Event - task: %s, msg: %s, code: %d\n",
            aResult.uid().c_str(),
            aResult.eventLog().message().c_str(),
            aResult.eventLog().code()
        );
    }


    // =================================================
    // DEBUG
    // =================================================

    if (
        aResult.isDebug()
    )
    {

        Firebase.printf(
            "Firebase Debug - task: %s, msg: %s\n",
            aResult.uid().c_str(),
            aResult.debug().c_str()
        );
    }


    // =================================================
    // ERROR
    // =================================================

    if (
        aResult.isError()
    )
    {

        Firebase.printf(
            "Firebase Error - task: %s, msg: %s, code: %d\n",
            aResult.uid().c_str(),
            aResult.error().message().c_str(),
            aResult.error().code()
        );
    }


    // =================================================
    // PAYLOAD
    // =================================================

    if (
        aResult.available()
    )
    {

        Firebase.printf(
            "Firebase Payload - task: %s, payload: %s\n",
            aResult.uid().c_str(),
            aResult.c_str()
        );
    }
}


// =====================================================
// FIREBASE SETUP
// =====================================================

void setupFirebase()
{

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "         FIREBASE SETUP"
    );

    Serial.println(
        "================================="
    );


    ssl_client.setInsecure();


    Serial.println(
        "Initializing Firebase..."
    );


    initializeApp(
        aClient,
        app,
        getAuth(user_auth),
        processFirebase,
        "authTask"
    );


    app.getApp<RealtimeDatabase>(
        Database
    );


    Database.url(
        DATABASE_URL
    );


    Serial.println(
        "Firebase initialization started."
    );


    Serial.println();
}


// =====================================================
// DATE
// =====================================================

String getDateString()
{

    struct tm timeinfo;


    if (
        !getLocalTime(
            &timeinfo
        )
    )
    {

        return "1970-01-01";
    }


    char buffer[20];


    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d",
        &timeinfo
    );


    return String(
        buffer
    );
}


// =====================================================
// TIME
// =====================================================

String getTimeString()
{

    struct tm timeinfo;


    if (
        !getLocalTime(
            &timeinfo
        )
    )
    {

        return "00:00:00";
    }


    char buffer[20];


    strftime(
        buffer,
        sizeof(buffer),
        "%H:%M:%S",
        &timeinfo
    );


    return String(
        buffer
    );
}


// =====================================================
// SEND SENSOR DATA
// =====================================================

void sendSensorData()
{

    // =================================================
    // FIREBASE READY
    // =================================================

    if (
        !app.ready()
    )
    {

        Serial.println();

        Serial.println(
            "Firebase not ready yet..."
        );

        return;
    }


    // =================================================
    // DHT11 READING
    // =================================================

    float humidity =
        dht.readHumidity();


    float temperature =
        dht.readTemperature();


    // =================================================
    // CHECK SENSOR
    // =================================================

    if (
        isnan(humidity) ||
        isnan(temperature)
    )
    {

        Serial.println();

        Serial.println(
            "================================="
        );

        Serial.println(
            "ERROR: Failed to read DHT11"
        );

        Serial.println(
            "================================="
        );

        return;
    }


    // =================================================
    // DATE / TIME
    // =================================================

    String date =
        getDateString();


    String time =
        getTimeString();


    // =================================================
    // BASE PATH
    // =================================================

    String basePath =
        "/ESP32_Data/" +
        date +
        "/" +
        time;


    // =================================================
    // SENSOR PATH
    // =================================================

    String temperaturePath =
        basePath +
        "/temperature";


    String humidityPath =
        basePath +
        "/humidity";


    // =================================================
    // SERIAL MONITOR
    // =================================================

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "       DHT11 SENSOR READING"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "Temperature: "
    );

    Serial.print(
        temperature,
        1
    );

    Serial.println(
        " °C"
    );


    Serial.print(
        "Humidity: "
    );

    Serial.print(
        humidity,
        1
    );

    Serial.println(
        " %"
    );


    Serial.print(
        "Date: "
    );

    Serial.println(
        date
    );


    Serial.print(
        "Time: "
    );

    Serial.println(
        time
    );


    Serial.print(
        "Firebase base path: "
    );

    Serial.println(
        basePath
    );


    Serial.println();


    // =================================================
    // FIREBASE WRITE
    // =================================================

    Database.set<float>(
        aClient,
        temperaturePath,
        temperature,
        processFirebase,
        "temperatureTask"
    );


    Database.set<float>(
        aClient,
        humidityPath,
        humidity,
        processFirebase,
        "humidityTask"
    );


    // =================================================
    // SERIAL CONFIRMATION
    // =================================================

    Serial.println(
        "Temperature write task sent."
    );

    Serial.println(
        "Humidity write task sent."
    );


    Serial.println();

    Serial.println(
        "Firebase paths:"
    );


    Serial.print(
        "Temperature: "
    );

    Serial.println(
        temperaturePath
    );


    Serial.print(
        "Humidity: "
    );

    Serial.println(
        humidityPath
    );


    Serial.println(
        "================================="
    );

    Serial.println();
}


// =====================================================
// SETUP
// =====================================================

void setup()
{

    Serial.begin(
        115200
    );


    delay(
        1000
    );


    Serial.println();

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "      LEAA ACTIVITY 4"
    );

    Serial.println(
        "     DHT11 FIREBASE MONITOR"
    );

    Serial.println(
        "================================="
    );


    // =================================================
    // LITTLEFS
    // =================================================

    Serial.println();

    Serial.println(
        "Starting LittleFS..."
    );


    if (
        !LittleFS.begin(true)
    )
    {

        Serial.println(
            "LittleFS mount failed!"
        );


        while (true)
        {
            delay(1000);
        }
    }


    Serial.println(
        "LittleFS ready."
    );


    // =================================================
    // DHT11
    // =================================================

    dht.begin();


    Serial.println(
        "DHT11 initialized."
    );


    Serial.print(
        "DHT11 GPIO: "
    );

    Serial.println(
        DHTPIN
    );


    // =================================================
    // WIFI
    // =================================================

    bool connected =
        connectToSavedWiFi();


    if (!connected)
    {

        startWiFiManager();

        return;
    }


    // =================================================
    // NTP
    // PHILIPPINES UTC+8
    // =================================================

    Serial.println(
        "Starting NTP time..."
    );


    configTime(
        8 * 3600,
        0,
        "pool.ntp.org",
        "time.nist.gov",
        "time.google.com"
    );


    Serial.print(
        "Waiting for time"
    );


    struct tm timeinfo;

    int retry = 0;


    while (
        !getLocalTime(
            &timeinfo
        ) &&
        retry < 20
    )
    {

        Serial.print(
            "."
        );

        delay(
            500
        );

        retry++;
    }


    Serial.println();


    if (
        getLocalTime(
            &timeinfo
        )
    )
    {

        Serial.println(
            "Time synchronized."
        );


        Serial.print(
            "Date: "
        );

        Serial.println(
            getDateString()
        );


        Serial.print(
            "Time: "
        );

        Serial.println(
            getTimeString()
        );

    }
    else
    {

        Serial.println(
            "WARNING: Time synchronization failed."
        );
    }


    // =================================================
    // FIREBASE
    // =================================================

    setupFirebase();


    // =================================================
    // WEB SERVER
    // =================================================

    startMainWebServer();


    // =================================================
    // READY
    // =================================================

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "          SYSTEM READY"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "Website: http://"
    );

    Serial.println(
        WiFi.localIP()
    );


    Serial.println();

    Serial.println(
        "DHT11 collection interval: 10 seconds"
    );

    Serial.println();
}


// =====================================================
// LOOP
// =====================================================

void loop()
{

    // =================================================
    // FIREBASE TASKS
    // =================================================

    if (
        !wifiManagerMode
    )
    {

        app.loop();
    }


    // =================================================
    // SENSOR EVERY 10 SECONDS
    // =================================================

    if (
        !wifiManagerMode &&
        millis() - lastSensorRead >=
        SENSOR_INTERVAL
    )
    {

        lastSensorRead =
            millis();


        sendSensorData();
    }


    delay(
        10
    );
}