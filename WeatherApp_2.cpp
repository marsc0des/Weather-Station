#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/html/htmlwin.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Define a structure to hold weather data
struct WeatherData {
    std::string city;
    std::string temperature;
    std::string description;
    std::string humidity;
    std::string windSpeed;
};

// Main application class
class WeatherApp : public wxApp {
public:
    virtual bool OnInit();
};

class WeatherFrame : public wxFrame {
public:
    WeatherFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

private:
    wxTextCtrl* textCtrl;
    wxHtmlWindow* htmlWindow;

    void OnGetWeather(wxCommandEvent& event);
    void FetchWeatherData(const wxString& city);
};

IMPLEMENT_APP(WeatherApp)

bool WeatherApp::OnInit() {
    WeatherFrame* frame = new WeatherFrame("Weather Station", wxPoint(50, 50), wxSize(600, 400));
    frame->Show(true);
    return true;
}

WeatherFrame::WeatherFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxFrame(NULL, wxID_ANY, title, pos, size) {

    wxPanel* panel = new wxPanel(this, wxID_ANY);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    textCtrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    htmlWindow = new wxHtmlWindow(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    mainSizer->Add(textCtrl, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(htmlWindow, 1, wxEXPAND | wxALL, 5);

    panel->SetSizer(mainSizer);

    Connect(textCtrl->GetId(), wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(WeatherFrame::OnGetWeather));

    Centre();
}

void WeatherFrame::OnGetWeather(wxCommandEvent& event) {
    wxString city = textCtrl->GetValue();
    if (!city.IsEmpty()) {
        FetchWeatherData(city);
    }
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

void WeatherFrame::FetchWeatherData(const wxString& city) {
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        std::string url = "http://api.openweathermap.org/data/2.5/weather?q=" + city.ToStdString() +
                          "&appid=//paste api key here//";

        std::string buffer;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            wxMessageBox("Failed to fetch weather data.", "Error", wxICON_ERROR | wxOK, this);
        } else {
            // Print JSON data to the console for debugging
            std::cout << buffer << std::endl;

            json jsonData = json::parse(buffer);

            WeatherData weatherData;
            weatherData.city = jsonData["name"].get<std::string>();
            weatherData.temperature = std::to_string(jsonData["main"]["temp"].get<double>());
            weatherData.description = jsonData["weather"][0]["description"].get<std::string>();
            weatherData.humidity = std::to_string(jsonData["main"]["humidity"].get<int>());
            weatherData.windSpeed = std::to_string(jsonData["wind"]["speed"].get<double>());

            wxString result;
            result << "<html><body>"
                   << "<h2>Weather Information for " << weatherData.city << "</h2>"
                   << "<p>Temperature: " << weatherData.temperature << "°C</p>"
                   << "<p>Description: " << weatherData.description << "</p>"
                   << "<p>Humidity: " << weatherData.humidity << "%</p>"
                   << "<p>Wind Speed: " << weatherData.windSpeed << " m/s</p>"
                   << "</body></html>";

            htmlWindow->SetPage(result);
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

