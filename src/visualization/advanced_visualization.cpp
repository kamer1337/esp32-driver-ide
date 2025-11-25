#include "visualization/advanced_visualization.h"
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>

namespace esp32_ide {
namespace visualization {

// ============================================================================
// SignalAnalyzer Implementation
// ============================================================================

SignalAnalyzer::SignalAnalyzer()
    : capturing_(false), trigger_level_(0.0), trigger_rising_(true),
      trigger_channel_(-1), next_channel_id_(1) {}

int SignalAnalyzer::AddChannel(const WaveformConfig& config) {
    int id = next_channel_id_++;
    channels_[id] = config;
    samples_[id] = std::vector<SignalSample>();
    return id;
}

bool SignalAnalyzer::RemoveChannel(int channel_id) {
    if (channels_.find(channel_id) == channels_.end()) return false;
    channels_.erase(channel_id);
    samples_.erase(channel_id);
    callbacks_.erase(channel_id);
    return true;
}

void SignalAnalyzer::ClearAllChannels() {
    channels_.clear();
    samples_.clear();
    callbacks_.clear();
}

WaveformConfig SignalAnalyzer::GetChannelConfig(int channel_id) const {
    auto it = channels_.find(channel_id);
    if (it != channels_.end()) {
        return it->second;
    }
    return WaveformConfig();
}

std::vector<int> SignalAnalyzer::GetChannelIds() const {
    std::vector<int> ids;
    for (const auto& pair : channels_) {
        ids.push_back(pair.first);
    }
    return ids;
}

void SignalAnalyzer::StartCapture() {
    capturing_ = true;
}

void SignalAnalyzer::StopCapture() {
    capturing_ = false;
}

void SignalAnalyzer::AddSample(int channel_id, const SignalSample& sample) {
    if (!capturing_) return;
    
    auto it = samples_.find(channel_id);
    if (it != samples_.end()) {
        it->second.push_back(sample);
        
        auto cb_it = callbacks_.find(channel_id);
        if (cb_it != callbacks_.end() && cb_it->second) {
            cb_it->second(sample);
        }
    }
}

std::vector<SignalSample> SignalAnalyzer::GetSamples(int channel_id, 
                                                       double start_time, 
                                                       double end_time) const {
    std::vector<SignalSample> result;
    auto it = samples_.find(channel_id);
    if (it != samples_.end()) {
        for (const auto& sample : it->second) {
            if (sample.timestamp_us >= start_time && sample.timestamp_us <= end_time) {
                result.push_back(sample);
            }
        }
    }
    return result;
}

void SignalAnalyzer::ClearSamples(int channel_id) {
    auto it = samples_.find(channel_id);
    if (it != samples_.end()) {
        it->second.clear();
    }
}

std::string SignalAnalyzer::GenerateWaveformASCII(int channel_id, int width, int height) const {
    std::ostringstream output;
    auto it = samples_.find(channel_id);
    if (it == samples_.end() || it->second.empty()) {
        return "No data available";
    }
    
    const auto& data = it->second;
    double min_val = data[0].value;
    double max_val = data[0].value;
    
    for (const auto& s : data) {
        min_val = std::min(min_val, s.value);
        max_val = std::max(max_val, s.value);
    }
    
    double range = max_val - min_val;
    if (range == 0) range = 1;
    
    std::vector<std::string> grid(height, std::string(width, ' '));
    
    for (size_t i = 0; i < data.size() && i < static_cast<size_t>(width); ++i) {
        int y = static_cast<int>((data[i].value - min_val) / range * (height - 1));
        y = height - 1 - y;
        if (y >= 0 && y < height) {
            grid[y][i] = '*';
        }
    }
    
    for (const auto& row : grid) {
        output << row << "\n";
    }
    
    return output.str();
}

std::string SignalAnalyzer::GenerateWaveformSVG(int channel_id, int width, int height) const {
    std::ostringstream svg;
    auto it = samples_.find(channel_id);
    
    svg << "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
    svg << "  <rect width=\"100%\" height=\"100%\" fill=\"#1E1E1E\"/>\n";
    
    if (it == samples_.end() || it->second.empty()) {
        svg << "  <text x=\"50%\" y=\"50%\" fill=\"white\" text-anchor=\"middle\">No data</text>\n";
        svg << "</svg>";
        return svg.str();
    }
    
    const auto& data = it->second;
    double min_val = data[0].value;
    double max_val = data[0].value;
    
    for (const auto& s : data) {
        min_val = std::min(min_val, s.value);
        max_val = std::max(max_val, s.value);
    }
    
    double range = max_val - min_val;
    if (range == 0) range = 1;
    
    svg << "  <polyline fill=\"none\" stroke=\"#00FF00\" stroke-width=\"1\" points=\"";
    
    for (size_t i = 0; i < data.size(); ++i) {
        double x = static_cast<double>(i) / data.size() * width;
        double y = height - ((data[i].value - min_val) / range * height);
        svg << x << "," << y << " ";
    }
    
    svg << "\"/>\n</svg>";
    return svg.str();
}

double SignalAnalyzer::GetFrequency(int channel_id) const {
    auto it = samples_.find(channel_id);
    if (it == samples_.end() || it->second.size() < 3) return 0.0;
    
    const auto& data = it->second;
    double avg = GetAverage(channel_id);
    
    int crossings = 0;
    for (size_t i = 1; i < data.size(); ++i) {
        if ((data[i-1].value < avg && data[i].value >= avg) ||
            (data[i-1].value >= avg && data[i].value < avg)) {
            crossings++;
        }
    }
    
    if (crossings < 2) return 0.0;
    
    double total_time = data.back().timestamp_us - data.front().timestamp_us;
    return (crossings / 2.0) / (total_time / 1000000.0);
}

double SignalAnalyzer::GetDutyCycle(int channel_id) const {
    auto it = samples_.find(channel_id);
    if (it == samples_.end() || it->second.empty()) return 0.0;
    
    const auto& data = it->second;
    double avg = GetAverage(channel_id);
    
    int high_count = 0;
    for (const auto& s : data) {
        if (s.value > avg) high_count++;
    }
    
    return static_cast<double>(high_count) / data.size() * 100.0;
}

double SignalAnalyzer::GetRMS(int channel_id) const {
    auto it = samples_.find(channel_id);
    if (it == samples_.end() || it->second.empty()) return 0.0;
    
    double sum_sq = 0.0;
    for (const auto& s : it->second) {
        sum_sq += s.value * s.value;
    }
    
    return std::sqrt(sum_sq / it->second.size());
}

double SignalAnalyzer::GetPeakToPeak(int channel_id) const {
    auto it = samples_.find(channel_id);
    if (it == samples_.end() || it->second.empty()) return 0.0;
    
    double min_val = it->second[0].value;
    double max_val = it->second[0].value;
    
    for (const auto& s : it->second) {
        min_val = std::min(min_val, s.value);
        max_val = std::max(max_val, s.value);
    }
    
    return max_val - min_val;
}

double SignalAnalyzer::GetAverage(int channel_id) const {
    auto it = samples_.find(channel_id);
    if (it == samples_.end() || it->second.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& s : it->second) {
        sum += s.value;
    }
    
    return sum / it->second.size();
}

void SignalAnalyzer::SetTriggerLevel(double level) {
    trigger_level_ = level;
}

void SignalAnalyzer::SetTriggerEdge(bool rising) {
    trigger_rising_ = rising;
}

void SignalAnalyzer::SetTriggerChannel(int channel_id) {
    trigger_channel_ = channel_id;
}

void SignalAnalyzer::SetSampleCallback(int channel_id, SampleCallback callback) {
    callbacks_[channel_id] = callback;
}

// ============================================================================
// LogicAnalyzer Implementation
// ============================================================================

LogicAnalyzer::LogicAnalyzer()
    : sample_rate_hz_(1000000.0), capturing_(false), next_channel_id_(1) {}

int LogicAnalyzer::AddChannel(const std::string& name, const std::string& color) {
    int id = next_channel_id_++;
    LogicChannel channel;
    channel.channel_id = id;
    channel.name = name;
    channel.color = color;
    channel.inverted = false;
    channels_[id] = channel;
    return id;
}

bool LogicAnalyzer::RemoveChannel(int channel_id) {
    return channels_.erase(channel_id) > 0;
}

void LogicAnalyzer::SetChannelInverted(int channel_id, bool inverted) {
    auto it = channels_.find(channel_id);
    if (it != channels_.end()) {
        it->second.inverted = inverted;
    }
}

void LogicAnalyzer::StartCapture(double sample_rate_hz) {
    sample_rate_hz_ = sample_rate_hz;
    capturing_ = true;
    for (auto& pair : channels_) {
        pair.second.samples.clear();
    }
}

void LogicAnalyzer::StopCapture() {
    capturing_ = false;
}

void LogicAnalyzer::RecordSample(int channel_id, bool value, double timestamp_us) {
    if (!capturing_) return;
    
    auto it = channels_.find(channel_id);
    if (it != channels_.end()) {
        SignalSample sample;
        sample.timestamp_us = timestamp_us;
        sample.value = value ? 1.0 : 0.0;
        sample.is_digital_high = it->second.inverted ? !value : value;
        it->second.samples.push_back(sample);
    }
}

std::vector<SignalSample> LogicAnalyzer::GetChannelData(int channel_id) const {
    auto it = channels_.find(channel_id);
    if (it != channels_.end()) {
        return it->second.samples;
    }
    return {};
}

std::vector<LogicAnalyzer::TimingMeasurement> LogicAnalyzer::MeasureTiming(int channel_id) const {
    std::vector<TimingMeasurement> measurements;
    auto it = channels_.find(channel_id);
    if (it == channels_.end() || it->second.samples.empty()) return measurements;
    
    TimingMeasurement freq;
    freq.name = "Frequency";
    freq.value_us = MeasureFrequency(channel_id);
    freq.description = "Hz";
    measurements.push_back(freq);
    
    TimingMeasurement high_pulse;
    high_pulse.name = "High Pulse Width";
    high_pulse.value_us = MeasurePulseWidth(channel_id, true);
    high_pulse.description = "microseconds";
    measurements.push_back(high_pulse);
    
    TimingMeasurement low_pulse;
    low_pulse.name = "Low Pulse Width";
    low_pulse.value_us = MeasurePulseWidth(channel_id, false);
    low_pulse.description = "microseconds";
    measurements.push_back(low_pulse);
    
    return measurements;
}

double LogicAnalyzer::MeasurePulseWidth(int channel_id, bool high_pulse) const {
    auto it = channels_.find(channel_id);
    if (it == channels_.end() || it->second.samples.size() < 2) return 0.0;
    
    const auto& samples = it->second.samples;
    double total_width = 0.0;
    int count = 0;
    
    for (size_t i = 1; i < samples.size(); ++i) {
        bool current_high = samples[i].is_digital_high;
        bool prev_high = samples[i-1].is_digital_high;
        
        if (current_high == high_pulse && prev_high == high_pulse) {
            total_width += samples[i].timestamp_us - samples[i-1].timestamp_us;
            count++;
        }
    }
    
    return count > 0 ? total_width / count : 0.0;
}

double LogicAnalyzer::MeasureFrequency(int channel_id) const {
    auto it = channels_.find(channel_id);
    if (it == channels_.end() || it->second.samples.size() < 3) return 0.0;
    
    const auto& samples = it->second.samples;
    int edges = 0;
    
    for (size_t i = 1; i < samples.size(); ++i) {
        if (samples[i].is_digital_high != samples[i-1].is_digital_high) {
            edges++;
        }
    }
    
    if (edges < 2) return 0.0;
    
    double total_time = samples.back().timestamp_us - samples.front().timestamp_us;
    return (edges / 2.0) / (total_time / 1000000.0);
}

std::vector<double> LogicAnalyzer::FindPattern(int channel_id, 
                                                 const std::vector<bool>& pattern) const {
    std::vector<double> positions;
    auto it = channels_.find(channel_id);
    if (it == channels_.end() || pattern.empty()) return positions;
    
    const auto& samples = it->second.samples;
    if (samples.size() < pattern.size()) return positions;
    
    for (size_t i = 0; i <= samples.size() - pattern.size(); ++i) {
        bool match = true;
        for (size_t j = 0; j < pattern.size() && match; ++j) {
            if (samples[i + j].is_digital_high != pattern[j]) {
                match = false;
            }
        }
        if (match) {
            positions.push_back(samples[i].timestamp_us);
        }
    }
    
    return positions;
}

std::string LogicAnalyzer::GenerateTimingDiagram(int width) const {
    std::ostringstream diagram;
    
    for (const auto& pair : channels_) {
        const auto& channel = pair.second;
        diagram << channel.name << ": ";
        
        if (channel.samples.empty()) {
            diagram << std::string(width, '-') << "\n";
            continue;
        }
        
        double total_time = channel.samples.back().timestamp_us - channel.samples.front().timestamp_us;
        if (total_time <= 0) total_time = 1;
        
        std::string line(width, ' ');
        for (size_t i = 0; i < channel.samples.size(); ++i) {
            double rel_pos = (channel.samples[i].timestamp_us - channel.samples.front().timestamp_us) / total_time;
            int x = static_cast<int>(rel_pos * (width - 1));
            if (x >= 0 && x < width) {
                line[x] = channel.samples[i].is_digital_high ? '-' : '_';
            }
        }
        
        diagram << line << "\n";
    }
    
    return diagram.str();
}

// ============================================================================
// ProtocolDecoder Implementation
// ============================================================================

ProtocolDecoder::ProtocolDecoder() {
    i2c_config_.sda_channel = 0;
    i2c_config_.scl_channel = 1;
    i2c_config_.show_ack_nack = true;
    
    spi_config_.mosi_channel = 0;
    spi_config_.miso_channel = 1;
    spi_config_.sck_channel = 2;
    spi_config_.cs_channel = 3;
    spi_config_.cpol = false;
    spi_config_.cpha = false;
    spi_config_.bits_per_word = 8;
    
    uart_config_.rx_channel = 0;
    uart_config_.tx_channel = 1;
    uart_config_.baud_rate = 115200;
    uart_config_.data_bits = 8;
    uart_config_.stop_bits = 1;
    uart_config_.parity = false;
    uart_config_.parity_odd = false;
}

void ProtocolDecoder::ConfigureI2C(const I2CConfig& config) {
    i2c_config_ = config;
}

void ProtocolDecoder::ConfigureSPI(const SPIConfig& config) {
    spi_config_ = config;
}

void ProtocolDecoder::ConfigureUART(const UARTConfig& config) {
    uart_config_ = config;
}

std::vector<DecodedMessage> ProtocolDecoder::DecodeI2C(
    const std::vector<SignalSample>& sda,
    const std::vector<SignalSample>& scl) const {
    
    std::vector<DecodedMessage> messages;
    if (sda.empty() || scl.empty()) return messages;
    
    // Simplified I2C decoding simulation
    DecodedMessage msg;
    msg.protocol = ProtocolType::I2C;
    msg.start_time_us = sda.front().timestamp_us;
    msg.end_time_us = sda.back().timestamp_us;
    msg.raw_data = "0xA0 0x00 0x55";
    msg.decoded_data = "Address: 0x50 (Write), Data: 0x00 0x55";
    msg.description = "I2C Write transaction";
    msg.is_valid = true;
    messages.push_back(msg);
    
    return messages;
}

std::vector<DecodedMessage> ProtocolDecoder::DecodeSPI(
    const std::vector<SignalSample>& mosi,
    const std::vector<SignalSample>& /*miso*/,
    const std::vector<SignalSample>& /*sck*/,
    const std::vector<SignalSample>& /*cs*/) const {
    
    std::vector<DecodedMessage> messages;
    if (mosi.empty()) return messages;
    
    // Simplified SPI decoding simulation
    DecodedMessage msg;
    msg.protocol = ProtocolType::SPI;
    msg.start_time_us = mosi.front().timestamp_us;
    msg.end_time_us = mosi.back().timestamp_us;
    msg.raw_data = "0x9F 0x00 0x00 0x00";
    msg.decoded_data = "JEDEC ID Read: Manufacturer=0xEF, Device=0x4016";
    msg.description = "SPI Flash JEDEC ID command";
    msg.is_valid = true;
    messages.push_back(msg);
    
    return messages;
}

std::vector<DecodedMessage> ProtocolDecoder::DecodeUART(
    const std::vector<SignalSample>& data) const {
    
    std::vector<DecodedMessage> messages;
    if (data.empty()) return messages;
    
    // Simplified UART decoding simulation
    DecodedMessage msg;
    msg.protocol = ProtocolType::UART;
    msg.start_time_us = data.front().timestamp_us;
    msg.end_time_us = data.back().timestamp_us;
    msg.raw_data = "0x48 0x65 0x6C 0x6C 0x6F";
    msg.decoded_data = "Hello";
    msg.description = "UART " + std::to_string(uart_config_.baud_rate) + " baud";
    msg.is_valid = true;
    messages.push_back(msg);
    
    return messages;
}

ProtocolType ProtocolDecoder::AutoDetectProtocol(const std::vector<SignalSample>& /*data*/) const {
    // Simplified auto-detection
    return ProtocolType::UART;
}

int ProtocolDecoder::AutoDetectBaudRate(const std::vector<SignalSample>& data) const {
    if (data.size() < 10) return 115200;
    
    // Find minimum pulse width
    double min_pulse = 1e9;
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i].is_digital_high != data[i-1].is_digital_high) {
            double pulse = data[i].timestamp_us - data[i-1].timestamp_us;
            if (pulse > 0) min_pulse = std::min(min_pulse, pulse);
        }
    }
    
    // Estimate baud rate from minimum pulse width
    int estimated = static_cast<int>(1000000.0 / min_pulse);
    
    // Round to nearest standard baud rate
    std::vector<int> standard_rates = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
    int closest = standard_rates[0];
    int min_diff = std::abs(estimated - closest);
    
    for (int rate : standard_rates) {
        int diff = std::abs(estimated - rate);
        if (diff < min_diff) {
            min_diff = diff;
            closest = rate;
        }
    }
    
    return closest;
}

std::string ProtocolDecoder::GenerateDecodedReport(const std::vector<DecodedMessage>& messages) const {
    std::ostringstream report;
    report << "Protocol Decoder Report\n";
    report << "=======================\n\n";
    
    for (size_t i = 0; i < messages.size(); ++i) {
        const auto& msg = messages[i];
        report << "Message " << (i + 1) << ":\n";
        report << "  Protocol: ";
        switch (msg.protocol) {
            case ProtocolType::I2C: report << "I2C"; break;
            case ProtocolType::SPI: report << "SPI"; break;
            case ProtocolType::UART: report << "UART"; break;
            case ProtocolType::ONE_WIRE: report << "1-Wire"; break;
            case ProtocolType::CAN: report << "CAN"; break;
            case ProtocolType::MODBUS: report << "Modbus"; break;
            default: report << "Custom"; break;
        }
        report << "\n";
        report << "  Time: " << std::fixed << std::setprecision(2) 
               << msg.start_time_us << " - " << msg.end_time_us << " us\n";
        report << "  Raw: " << msg.raw_data << "\n";
        report << "  Decoded: " << msg.decoded_data << "\n";
        report << "  Description: " << msg.description << "\n";
        report << "  Valid: " << (msg.is_valid ? "Yes" : "No") << "\n\n";
    }
    
    return report.str();
}

bool ProtocolDecoder::DetectStartCondition(const std::vector<SignalSample>& sda,
                                            const std::vector<SignalSample>& scl,
                                            size_t index) const {
    if (index >= sda.size() || index >= scl.size()) return false;
    if (index == 0) return false;
    
    return sda[index-1].is_digital_high && !sda[index].is_digital_high && 
           scl[index].is_digital_high;
}

bool ProtocolDecoder::DetectStopCondition(const std::vector<SignalSample>& sda,
                                           const std::vector<SignalSample>& scl,
                                           size_t index) const {
    if (index >= sda.size() || index >= scl.size()) return false;
    if (index == 0) return false;
    
    return !sda[index-1].is_digital_high && sda[index].is_digital_high && 
           scl[index].is_digital_high;
}

// ============================================================================
// Visualization3D Implementation
// ============================================================================

Visualization3D::Visualization3D()
    : board_width_mm_(100.0f), board_height_mm_(80.0f), board_thickness_mm_(1.6f),
      board_color_("#2E7D32"), view_pitch_(30.0f), view_yaw_(45.0f), 
      view_roll_(0.0f), zoom_level_(1.0f) {}

void Visualization3D::SetBoardSize(float width_mm, float height_mm, float thickness_mm) {
    board_width_mm_ = width_mm;
    board_height_mm_ = height_mm;
    board_thickness_mm_ = thickness_mm;
}

void Visualization3D::SetBoardColor(const std::string& color) {
    board_color_ = color;
}

BoundingBox3D Visualization3D::GetBoardBounds() const {
    BoundingBox3D bounds;
    bounds.min = Point3D(0, 0, 0);
    bounds.max = Point3D(board_width_mm_, board_height_mm_, board_thickness_mm_);
    return bounds;
}

void Visualization3D::AddComponent(const ComponentFootprint& component) {
    components_[component.id] = component;
}

void Visualization3D::RemoveComponent(const std::string& id) {
    components_.erase(id);
}

void Visualization3D::MoveComponent(const std::string& id, const Point3D& new_position) {
    auto it = components_.find(id);
    if (it != components_.end()) {
        float dx = new_position.x - it->second.bounds.min.x;
        float dy = new_position.y - it->second.bounds.min.y;
        float dz = new_position.z - it->second.bounds.min.z;
        
        it->second.bounds.min.x += dx;
        it->second.bounds.min.y += dy;
        it->second.bounds.min.z += dz;
        it->second.bounds.max.x += dx;
        it->second.bounds.max.y += dy;
        it->second.bounds.max.z += dz;
        
        for (auto& pin : it->second.pins) {
            pin.x += dx;
            pin.y += dy;
            pin.z += dz;
        }
    }
}

void Visualization3D::RotateComponent(const std::string& id, float angle_degrees) {
    auto it = components_.find(id);
    if (it != components_.end()) {
        float rad = angle_degrees * 3.14159265f / 180.0f;
        float cos_a = std::cos(rad);
        float sin_a = std::sin(rad);
        
        // Get center
        float cx = (it->second.bounds.min.x + it->second.bounds.max.x) / 2.0f;
        float cy = (it->second.bounds.min.y + it->second.bounds.max.y) / 2.0f;
        
        // Rotate pins around center
        for (auto& pin : it->second.pins) {
            float px = pin.x - cx;
            float py = pin.y - cy;
            pin.x = cx + px * cos_a - py * sin_a;
            pin.y = cy + px * sin_a + py * cos_a;
        }
    }
}

std::vector<ComponentFootprint> Visualization3D::GetAllComponents() const {
    std::vector<ComponentFootprint> result;
    for (const auto& pair : components_) {
        result.push_back(pair.second);
    }
    return result;
}

ComponentFootprint* Visualization3D::GetComponent(const std::string& id) {
    auto it = components_.find(id);
    return (it != components_.end()) ? &it->second : nullptr;
}

void Visualization3D::AddTrace(const PCBTrace& trace) {
    traces_.push_back(trace);
}

void Visualization3D::RemoveTrace(const std::string& net_name) {
    traces_.erase(
        std::remove_if(traces_.begin(), traces_.end(),
                      [&net_name](const PCBTrace& t) { return t.net_name == net_name; }),
        traces_.end()
    );
}

std::vector<PCBTrace> Visualization3D::GetTraces() const {
    return traces_;
}

void Visualization3D::AddVia(const Via& via) {
    vias_.push_back(via);
}

std::vector<Via> Visualization3D::GetVias() const {
    return vias_;
}

void Visualization3D::AutoPlaceComponents() {
    float x = 10.0f;
    float y = 10.0f;
    float max_height = 0.0f;
    
    for (auto& pair : components_) {
        float comp_width = pair.second.bounds.Width();
        float comp_height = pair.second.bounds.Height();
        
        if (x + comp_width > board_width_mm_ - 10.0f) {
            x = 10.0f;
            y += max_height + 5.0f;
            max_height = 0.0f;
        }
        
        MoveComponent(pair.first, Point3D(x, y, board_thickness_mm_));
        x += comp_width + 5.0f;
        max_height = std::max(max_height, comp_height);
    }
}

std::vector<std::string> Visualization3D::ValidatePlacement() const {
    std::vector<std::string> errors;
    
    for (auto it1 = components_.begin(); it1 != components_.end(); ++it1) {
        // Check bounds
        if (it1->second.bounds.min.x < 0 || it1->second.bounds.max.x > board_width_mm_ ||
            it1->second.bounds.min.y < 0 || it1->second.bounds.max.y > board_height_mm_) {
            errors.push_back("Component " + it1->first + " is outside board bounds");
        }
        
        // Check collisions
        for (auto it2 = std::next(it1); it2 != components_.end(); ++it2) {
            if (CheckCollision(it1->second, it2->second)) {
                errors.push_back("Collision between " + it1->first + " and " + it2->first);
            }
        }
    }
    
    return errors;
}

float Visualization3D::CalculateRoutingDensity() const {
    float total_length = 0.0f;
    for (const auto& trace : traces_) {
        total_length += CalculateWireLength(trace);
    }
    
    float board_area = board_width_mm_ * board_height_mm_;
    return total_length / board_area;
}

std::vector<Point3D> Visualization3D::AutoRouteNet(const std::string& /*net_name*/,
                                                     const std::vector<std::string>& pin_ids) {
    std::vector<Point3D> path;
    
    // Simplified Manhattan routing
    for (const auto& pin_id : pin_ids) {
        for (const auto& comp : components_) {
            for (const auto& pin : comp.second.pins) {
                // Add waypoints
                path.push_back(pin);
            }
        }
    }
    
    return path;
}

float Visualization3D::CalculateWireLength(const PCBTrace& trace) const {
    float length = 0.0f;
    for (size_t i = 1; i < trace.path.size(); ++i) {
        float dx = trace.path[i].x - trace.path[i-1].x;
        float dy = trace.path[i].y - trace.path[i-1].y;
        length += std::sqrt(dx*dx + dy*dy);
    }
    return length;
}

std::string Visualization3D::GenerateSVG(PCBLayer /*layer*/, int width, int height) const {
    std::ostringstream svg;
    
    float scale_x = static_cast<float>(width) / board_width_mm_;
    float scale_y = static_cast<float>(height) / board_height_mm_;
    float scale = std::min(scale_x, scale_y);
    
    svg << "<svg width=\"" << width << "\" height=\"" << height 
        << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
    
    // Board
    svg << "  <rect x=\"0\" y=\"0\" width=\"" << board_width_mm_ * scale 
        << "\" height=\"" << board_height_mm_ * scale 
        << "\" fill=\"" << board_color_ << "\" stroke=\"#1B5E20\"/>\n";
    
    // Components
    for (const auto& pair : components_) {
        const auto& comp = pair.second;
        svg << "  <rect x=\"" << comp.bounds.min.x * scale 
            << "\" y=\"" << comp.bounds.min.y * scale
            << "\" width=\"" << comp.bounds.Width() * scale 
            << "\" height=\"" << comp.bounds.Height() * scale
            << "\" fill=\"" << comp.color << "\" stroke=\"#000\"/>\n";
    }
    
    // Traces
    for (const auto& trace : traces_) {
        svg << "  <polyline fill=\"none\" stroke=\"" << trace.color 
            << "\" stroke-width=\"" << trace.width_mm * scale << "\" points=\"";
        for (const auto& p : trace.path) {
            svg << p.x * scale << "," << p.y * scale << " ";
        }
        svg << "\"/>\n";
    }
    
    svg << "</svg>";
    return svg.str();
}

std::string Visualization3D::Generate3DOBJ() const {
    std::ostringstream obj;
    
    obj << "# ESP32 Driver IDE PCB Model\n";
    obj << "# Board: " << board_width_mm_ << "mm x " << board_height_mm_ << "mm\n\n";
    
    // Board vertices
    obj << "v 0 0 0\n";
    obj << "v " << board_width_mm_ << " 0 0\n";
    obj << "v " << board_width_mm_ << " " << board_height_mm_ << " 0\n";
    obj << "v 0 " << board_height_mm_ << " 0\n";
    obj << "v 0 0 " << board_thickness_mm_ << "\n";
    obj << "v " << board_width_mm_ << " 0 " << board_thickness_mm_ << "\n";
    obj << "v " << board_width_mm_ << " " << board_height_mm_ << " " << board_thickness_mm_ << "\n";
    obj << "v 0 " << board_height_mm_ << " " << board_thickness_mm_ << "\n";
    
    // Board faces
    obj << "\n# Board faces\n";
    obj << "f 1 2 3 4\n";
    obj << "f 5 6 7 8\n";
    obj << "f 1 2 6 5\n";
    obj << "f 2 3 7 6\n";
    obj << "f 3 4 8 7\n";
    obj << "f 4 1 5 8\n";
    
    return obj.str();
}

std::string Visualization3D::GenerateGerber(PCBLayer layer) const {
    std::ostringstream gerber;
    
    gerber << "G04 ESP32 Driver IDE Gerber Output*\n";
    gerber << "%FSLAX36Y36*%\n";
    gerber << "%MOIN*%\n";
    
    std::string layer_name;
    switch (layer) {
        case PCBLayer::TOP_COPPER: layer_name = "Top Copper"; break;
        case PCBLayer::BOTTOM_COPPER: layer_name = "Bottom Copper"; break;
        case PCBLayer::TOP_SILKSCREEN: layer_name = "Top Silkscreen"; break;
        case PCBLayer::BOTTOM_SILKSCREEN: layer_name = "Bottom Silkscreen"; break;
        default: layer_name = "Unknown"; break;
    }
    
    gerber << "G04 Layer: " << layer_name << "*\n";
    gerber << "M02*\n";
    
    return gerber.str();
}

void Visualization3D::SetViewAngle(float pitch, float yaw, float roll) {
    view_pitch_ = pitch;
    view_yaw_ = yaw;
    view_roll_ = roll;
}

void Visualization3D::SetZoom(float zoom_level) {
    zoom_level_ = zoom_level;
}

Point3D Visualization3D::GetCameraPosition() const {
    return Point3D(
        board_width_mm_ / 2.0f,
        board_height_mm_ / 2.0f,
        100.0f / zoom_level_
    );
}

std::vector<std::string> Visualization3D::CheckDesignRules(float min_trace_width_mm,
                                                            float min_clearance_mm) const {
    std::vector<std::string> violations;
    
    for (const auto& trace : traces_) {
        if (trace.width_mm < min_trace_width_mm) {
            violations.push_back("Trace " + trace.net_name + " width below minimum");
        }
    }
    
    // Check clearances between traces
    for (size_t i = 0; i < traces_.size(); ++i) {
        for (size_t j = i + 1; j < traces_.size(); ++j) {
            // Simplified clearance check
            if (traces_[i].layer == traces_[j].layer) {
                // Would need actual geometry check here
                (void)min_clearance_mm;  // Suppress unused warning
            }
        }
    }
    
    return violations;
}

float Visualization3D::EstimateBoardCost() const {
    float area_sqin = (board_width_mm_ * board_height_mm_) / 645.16f;  // mm² to in²
    float base_cost = area_sqin * 5.0f;  // $5 per sq inch base
    float component_cost = components_.size() * 0.10f;  // $0.10 per component placement
    float via_cost = vias_.size() * 0.02f;  // $0.02 per via
    
    return base_cost + component_cost + via_cost;
}

bool Visualization3D::CheckCollision(const ComponentFootprint& a, 
                                       const ComponentFootprint& b) const {
    return !(a.bounds.max.x < b.bounds.min.x || a.bounds.min.x > b.bounds.max.x ||
             a.bounds.max.y < b.bounds.min.y || a.bounds.min.y > b.bounds.max.y);
}

Point3D Visualization3D::Project2D(const Point3D& point3d) const {
    // Simplified orthographic projection
    float pitch_rad = view_pitch_ * 3.14159265f / 180.0f;
    float yaw_rad = view_yaw_ * 3.14159265f / 180.0f;
    
    float x = point3d.x * std::cos(yaw_rad) - point3d.y * std::sin(yaw_rad);
    float y = point3d.x * std::sin(yaw_rad) * std::sin(pitch_rad) + 
              point3d.y * std::cos(yaw_rad) * std::sin(pitch_rad) + 
              point3d.z * std::cos(pitch_rad);
    
    return Point3D(x * zoom_level_, y * zoom_level_, 0);
}

// ============================================================================
// NetworkVisualizer Implementation
// ============================================================================

NetworkVisualizer::NetworkVisualizer() : bluetooth_scanning_(false) {}

void NetworkVisualizer::AddNode(const NetworkNode& node) {
    nodes_[node.id] = node;
    NotifyNodeDiscovered(node);
}

void NetworkVisualizer::RemoveNode(const std::string& node_id) {
    nodes_.erase(node_id);
    
    // Remove associated connections
    connections_.erase(
        std::remove_if(connections_.begin(), connections_.end(),
                      [&node_id](const NetworkConnection& c) {
                          return c.source_id == node_id || c.target_id == node_id;
                      }),
        connections_.end()
    );
}

void NetworkVisualizer::UpdateNode(const NetworkNode& node) {
    nodes_[node.id] = node;
}

NetworkNode* NetworkVisualizer::GetNode(const std::string& node_id) {
    auto it = nodes_.find(node_id);
    return (it != nodes_.end()) ? &it->second : nullptr;
}

std::vector<NetworkNode> NetworkVisualizer::GetAllNodes() const {
    std::vector<NetworkNode> result;
    for (const auto& pair : nodes_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<NetworkNode> NetworkVisualizer::GetNodesByType(NetworkNodeType type) const {
    std::vector<NetworkNode> result;
    for (const auto& pair : nodes_) {
        if (pair.second.type == type) {
            result.push_back(pair.second);
        }
    }
    return result;
}

void NetworkVisualizer::AddConnection(const NetworkConnection& connection) {
    connections_.push_back(connection);
}

void NetworkVisualizer::RemoveConnection(const std::string& source_id, 
                                          const std::string& target_id) {
    connections_.erase(
        std::remove_if(connections_.begin(), connections_.end(),
                      [&source_id, &target_id](const NetworkConnection& c) {
                          return c.source_id == source_id && c.target_id == target_id;
                      }),
        connections_.end()
    );
}

std::vector<NetworkConnection> NetworkVisualizer::GetConnections() const {
    return connections_;
}

std::vector<NetworkConnection> NetworkVisualizer::GetNodeConnections(
    const std::string& node_id) const {
    std::vector<NetworkConnection> result;
    for (const auto& conn : connections_) {
        if (conn.source_id == node_id || conn.target_id == node_id) {
            result.push_back(conn);
        }
    }
    return result;
}

void NetworkVisualizer::ScanWiFiNetworks() {
    // Simulated WiFi scan
    WiFiNetwork home;
    home.ssid = "HomeNetwork";
    home.bssid = "AA:BB:CC:DD:EE:FF";
    home.channel = 6;
    home.signal_strength_dbm = -45;
    home.security_type = "WPA2";
    home.is_connected = true;
    AddWiFiNetwork(home);
    
    WiFiNetwork neighbor;
    neighbor.ssid = "NeighborWiFi";
    neighbor.bssid = "11:22:33:44:55:66";
    neighbor.channel = 11;
    neighbor.signal_strength_dbm = -72;
    neighbor.security_type = "WPA2";
    neighbor.is_connected = false;
    AddWiFiNetwork(neighbor);
}

void NetworkVisualizer::AddWiFiNetwork(const WiFiNetwork& network) {
    wifi_networks_.push_back(network);
    
    // Create node for network
    NetworkNode node;
    node.id = "wifi_" + network.bssid;
    node.name = network.ssid;
    node.type = NetworkNodeType::WIFI_ACCESS_POINT;
    node.mac_address = network.bssid;
    node.signal_strength_dbm = network.signal_strength_dbm;
    node.is_connected = network.is_connected;
    AddNode(node);
}

std::vector<WiFiNetwork> NetworkVisualizer::GetWiFiNetworks() const {
    return wifi_networks_;
}

std::string NetworkVisualizer::GenerateWiFiMap() const {
    std::ostringstream map;
    map << "WiFi Network Map\n";
    map << "================\n\n";
    
    for (const auto& network : wifi_networks_) {
        int bars = std::min(5, std::max(1, (network.signal_strength_dbm + 100) / 15));
        std::string signal(bars, '|');
        std::string empty(5 - bars, ' ');
        
        map << "[" << signal << empty << "] " << network.ssid;
        if (network.is_connected) map << " *";
        map << " (Ch " << network.channel << ", " << network.security_type << ")\n";
    }
    
    return map.str();
}

void NetworkVisualizer::StartBluetoothScan() {
    bluetooth_scanning_ = true;
    bluetooth_devices_.clear();
}

void NetworkVisualizer::StopBluetoothScan() {
    bluetooth_scanning_ = false;
}

void NetworkVisualizer::AddBluetoothDevice(const BluetoothDevice& device) {
    bluetooth_devices_.push_back(device);
    
    NetworkNode node;
    node.id = "bt_" + device.address;
    node.name = device.name;
    node.type = device.is_ble ? NetworkNodeType::BLE_DEVICE : NetworkNodeType::BLUETOOTH_DEVICE;
    node.mac_address = device.address;
    node.signal_strength_dbm = device.rssi;
    node.is_connected = device.is_connected;
    AddNode(node);
}

std::vector<BluetoothDevice> NetworkVisualizer::GetBluetoothDevices() const {
    return bluetooth_devices_;
}

std::string NetworkVisualizer::GenerateBluetoothMap() const {
    std::ostringstream map;
    map << "Bluetooth Device Map\n";
    map << "====================\n\n";
    
    for (const auto& device : bluetooth_devices_) {
        map << (device.is_ble ? "[BLE] " : "[BT]  ");
        map << device.name << " (" << device.address << ")";
        if (device.is_connected) map << " [Connected]";
        if (device.is_paired) map << " [Paired]";
        map << " RSSI: " << device.rssi << " dBm\n";
    }
    
    return map.str();
}

void NetworkVisualizer::AddMQTTTopic(const MQTTTopicNode& topic) {
    mqtt_topics_[topic.topic] = topic;
    
    // Update parent's child list
    if (!topic.parent_topic.empty()) {
        auto parent = mqtt_topics_.find(topic.parent_topic);
        if (parent != mqtt_topics_.end()) {
            parent->second.child_topics.push_back(topic.topic);
        }
    }
}

void NetworkVisualizer::RemoveMQTTTopic(const std::string& topic) {
    mqtt_topics_.erase(topic);
}

MQTTTopicNode* NetworkVisualizer::GetMQTTTopic(const std::string& topic) {
    auto it = mqtt_topics_.find(topic);
    return (it != mqtt_topics_.end()) ? &it->second : nullptr;
}

std::vector<MQTTTopicNode> NetworkVisualizer::GetMQTTTopics() const {
    std::vector<MQTTTopicNode> result;
    for (const auto& pair : mqtt_topics_) {
        result.push_back(pair.second);
    }
    return result;
}

std::string NetworkVisualizer::GenerateMQTTTopicTree() const {
    std::ostringstream tree;
    tree << "MQTT Topic Tree\n";
    tree << "===============\n\n";
    
    // Find root topics (no parent)
    for (const auto& pair : mqtt_topics_) {
        if (pair.second.parent_topic.empty()) {
            tree << pair.first << " (" << pair.second.subscriber_count << " subscribers)\n";
            
            // Print children
            for (const auto& child : pair.second.child_topics) {
                tree << "  +-- " << child << "\n";
            }
        }
    }
    
    return tree.str();
}

void NetworkVisualizer::UpdateMQTTActivity(const std::string& topic, 
                                            const std::string& message) {
    auto it = mqtt_topics_.find(topic);
    if (it != mqtt_topics_.end()) {
        it->second.last_message = message;
        it->second.last_activity = std::chrono::system_clock::now();
        it->second.message_rate_per_minute++;
    }
}

std::string NetworkVisualizer::GenerateNetworkDiagram() const {
    std::ostringstream diagram;
    diagram << "Network Topology Diagram\n";
    diagram << "========================\n\n";
    
    for (const auto& pair : nodes_) {
        diagram << "[" << pair.second.name << "]";
        
        auto conns = GetNodeConnections(pair.first);
        if (!conns.empty()) {
            diagram << " ---> ";
            for (size_t i = 0; i < conns.size(); ++i) {
                if (i > 0) diagram << ", ";
                std::string target = conns[i].source_id == pair.first ? 
                                    conns[i].target_id : conns[i].source_id;
                auto target_node = nodes_.find(target);
                if (target_node != nodes_.end()) {
                    diagram << target_node->second.name;
                }
            }
        }
        diagram << "\n";
    }
    
    return diagram.str();
}

std::string NetworkVisualizer::GenerateNetworkSVG(int width, int height) const {
    std::ostringstream svg;
    
    svg << "<svg width=\"" << width << "\" height=\"" << height 
        << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
    svg << "  <rect width=\"100%\" height=\"100%\" fill=\"#1E1E1E\"/>\n";
    
    // Draw connections
    for (const auto& conn : connections_) {
        auto src = nodes_.find(conn.source_id);
        auto dst = nodes_.find(conn.target_id);
        if (src != nodes_.end() && dst != nodes_.end()) {
            svg << "  <line x1=\"" << src->second.position.x 
                << "\" y1=\"" << src->second.position.y
                << "\" x2=\"" << dst->second.position.x 
                << "\" y2=\"" << dst->second.position.y
                << "\" stroke=\"#666\" stroke-width=\"2\"/>\n";
        }
    }
    
    // Draw nodes
    for (const auto& pair : nodes_) {
        std::string color = "#007ACC";
        switch (pair.second.type) {
            case NetworkNodeType::WIFI_ACCESS_POINT: color = "#4CAF50"; break;
            case NetworkNodeType::BLUETOOTH_DEVICE: color = "#2196F3"; break;
            case NetworkNodeType::MQTT_BROKER: color = "#FF9800"; break;
            default: break;
        }
        
        svg << "  <circle cx=\"" << pair.second.position.x 
            << "\" cy=\"" << pair.second.position.y 
            << "\" r=\"20\" fill=\"" << color << "\"/>\n";
        svg << "  <text x=\"" << pair.second.position.x 
            << "\" y=\"" << (pair.second.position.y + 35)
            << "\" fill=\"white\" text-anchor=\"middle\" font-size=\"12\">" 
            << pair.second.name << "</text>\n";
    }
    
    svg << "</svg>";
    return svg.str();
}

std::string NetworkVisualizer::GenerateTopologyJSON() const {
    std::ostringstream json;
    
    json << "{\n  \"nodes\": [\n";
    bool first = true;
    for (const auto& pair : nodes_) {
        if (!first) json << ",\n";
        first = false;
        json << "    {\"id\": \"" << pair.first << "\", \"name\": \"" << pair.second.name << "\"}";
    }
    json << "\n  ],\n  \"connections\": [\n";
    
    first = true;
    for (const auto& conn : connections_) {
        if (!first) json << ",\n";
        first = false;
        json << "    {\"source\": \"" << conn.source_id << "\", \"target\": \"" << conn.target_id << "\"}";
    }
    json << "\n  ]\n}";
    
    return json.str();
}

void NetworkVisualizer::ApplyForceDirectedLayout() {
    // Simplified force-directed layout
    float center_x = 400.0f;
    float center_y = 300.0f;
    float radius = 200.0f;
    
    int i = 0;
    int n = nodes_.size();
    for (auto& pair : nodes_) {
        float angle = 2.0f * 3.14159265f * i / n;
        pair.second.position.x = center_x + radius * std::cos(angle);
        pair.second.position.y = center_y + radius * std::sin(angle);
        i++;
    }
}

void NetworkVisualizer::ApplyHierarchicalLayout() {
    float y = 50.0f;
    float x_spacing = 150.0f;
    
    for (auto& pair : nodes_) {
        pair.second.position.x = (nodes_.size() > 1) ? 
            100.0f + x_spacing * (&pair - &*nodes_.begin()) : 400.0f;
        pair.second.position.y = y;
        y += 80.0f;
    }
}

void NetworkVisualizer::ApplyCircularLayout() {
    ApplyForceDirectedLayout();  // Same for now
}

std::vector<std::string> NetworkVisualizer::FindPath(const std::string& from_id,
                                                       const std::string& to_id) const {
    std::vector<std::string> path;
    
    // Simplified BFS
    if (from_id == to_id) {
        path.push_back(from_id);
        return path;
    }
    
    std::map<std::string, std::string> prev;
    std::vector<std::string> queue;
    queue.push_back(from_id);
    prev[from_id] = "";
    
    while (!queue.empty()) {
        std::string current = queue.front();
        queue.erase(queue.begin());
        
        if (current == to_id) {
            // Reconstruct path
            std::string node = to_id;
            while (!node.empty()) {
                path.insert(path.begin(), node);
                node = prev[node];
            }
            return path;
        }
        
        for (const auto& conn : connections_) {
            std::string neighbor;
            if (conn.source_id == current) neighbor = conn.target_id;
            else if (conn.target_id == current) neighbor = conn.source_id;
            
            if (!neighbor.empty() && prev.find(neighbor) == prev.end()) {
                prev[neighbor] = current;
                queue.push_back(neighbor);
            }
        }
    }
    
    return path;  // Empty if no path found
}

float NetworkVisualizer::CalculateNetworkDensity() const {
    if (nodes_.size() < 2) return 0.0f;
    
    float max_connections = nodes_.size() * (nodes_.size() - 1) / 2.0f;
    return connections_.size() / max_connections;
}

std::vector<std::string> NetworkVisualizer::FindBottlenecks() const {
    std::vector<std::string> bottlenecks;
    
    // Find nodes with many connections (potential bottlenecks)
    for (const auto& pair : nodes_) {
        auto conns = GetNodeConnections(pair.first);
        if (conns.size() > 3) {
            bottlenecks.push_back(pair.second.name + " (" + 
                                 std::to_string(conns.size()) + " connections)");
        }
    }
    
    return bottlenecks;
}

void NetworkVisualizer::NotifyNodeDiscovered(const NetworkNode& node) {
    if (node_callback_) {
        node_callback_(node);
    }
}

Point3D NetworkVisualizer::CalculateNodePosition(const std::string& node_id) const {
    auto it = nodes_.find(node_id);
    if (it != nodes_.end()) {
        return it->second.position;
    }
    return Point3D();
}

} // namespace visualization
} // namespace esp32_ide
