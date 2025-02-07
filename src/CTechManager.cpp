#include "CTechManager.h"

CTechManager::CTechManager(uint16_t addr)
{
    deviceAddress = addr;

    SetStatsDelay(180);
}

void CTechManager::Update()
{
    // Process data.
    if (ioStream != nullptr)
    {
        while(ioStream->available())
        {
            if (AppendByte(ioStream->read()))
            {
                // Send ACK.
                if (autoAck || (txSize > 2)) 
                    SendPacket();
            }
        }
    }

    // Stats.
    if (millis() - statsStamp > statsDelay)
    {
        statsStamp = millis();
        StoreStats();
    }
}

void CTechManager::GetStateJson(Print& output, bool raw)
{
    json.clear();

    if (raw)
        json["device_time"] = deviceState.device_time;
    else
        json["device_time"] = GetTime((int16_t)deviceState.device_time);

    json["device_day"] = deviceState.device_day;

    if (raw)
    {
        json["fumes_temp"]          = deviceState.fumes_temp;
        json["ext_temp"]            = deviceState.external_temp;

        json["co_temp"]             = deviceState.co_temp;
        json["co_temp_set"]         = deviceState.co_temp_set;
        json["co_temp_adj"]         = deviceState.co_temp_adj;
        json["co_min_max"]["min"]   = deviceState.co_min_max;
        json["co_min_max"]["max"]   = deviceState.co_min_max;

        json["cwu_temp"]            = deviceState.cwu_temp;
        json["cwu_temp_ret"]        = deviceState.cwu_temp_ret;
        json["cwu_temp_set"]        = deviceState.cwu_temp_set;
        json["cwu_min_max"]["min"]  = deviceState.cwu_min_max;
        json["cwu_min_max"]["max"]  = deviceState.cwu_min_max;

        json["fuel_stock_level"]    = deviceState.fuel_stock_level;
    }
    else
    {
        json["fumes_temp"]          = GetTempMeasured((int16_t)deviceState.fumes_temp);
        json["ext_temp"]            = GetTempMeasured((int16_t)deviceState.external_temp);

        json["co_temp"]             = GetTempMeasured((int16_t)deviceState.co_temp);
        json["co_temp_set"]         = GetTempSet((int16_t)deviceState.co_temp_set);
        json["co_temp_adj"]         = GetTempSet((int16_t)deviceState.co_temp_adj);
        json["co_min_max"]["min"]   = GetTempMin(deviceState.co_min_max);
        json["co_min_max"]["max"]   = GetTempMax(deviceState.co_min_max);

        json["cwu_temp"]            = GetTempMeasured((int16_t)deviceState.cwu_temp);
        json["cwu_temp_ret"]        = GetTempMeasured((int16_t)deviceState.cwu_temp_ret);
        json["cwu_temp_set"]        = GetTempSet((int16_t)deviceState.cwu_temp_set);
        json["cwu_min_max"]["min"]  = GetTempMin(deviceState.cwu_min_max);
        json["cwu_min_max"]["max"]  = GetTempMax(deviceState.cwu_min_max);

        json["fuel_stock_level"]    = GetPercentPrecise(deviceState.fuel_stock_level);
    }

    json["fuel_stock_time"]     = deviceState.fuel_stock_time;

    json["pump_mode"]           = deviceState.pump_mode;
    json["pump_state_co"]       = deviceState.pump_state_co;
    json["pump_state_cwu"]      = deviceState.pump_state_cwu;

    json["fan_state"]           = deviceState.fan_state;
    json["fan_speed"]           = deviceState.fan_speed;
    
    json["feeder_state"]        = deviceState.feeder_state;

    if (raw)
        json["feeder_temp"] = deviceState.feeder_temp;
    else
        json["feeder_temp"] = GetTempMeasured((int16_t)deviceState.feeder_temp);

    JsonArray valves = json.createNestedArray("valves");

    for (uint16_t i = 0; i < 4; i++)
    {
        JsonObject valve = valves.createNestedObject();

        valve["address"] = deviceState.valveData[i].address;
        valve["state"] = deviceState.valveData[i].state;
        valve["openLevel"] = deviceState.valveData[i].openLevel;
        valve["type"] = deviceState.valveData[i].type;

        if (raw)
        {
            valve["temp_set"] = deviceState.valveData[i].temp_set;
            valve["temp"] = deviceState.valveData[i].temp;
            valve["min"] = deviceState.valveData[i].minMax;
            valve["max"] = deviceState.valveData[i].minMax;
        }
        else 
        {
            valve["temp_set"] = GetTempSet((int16_t)deviceState.valveData[i].temp_set);
            valve["temp"] = GetTempMeasured((int16_t)deviceState.valveData[i].temp);
            valve["min"] = GetTempMax(deviceState.valveData[i].minMax);;
            valve["max"] = GetTempMax(deviceState.valveData[i].minMax);
        }
    }

    json["request_stamp"]        = requestStamp++;
    json["device_type"]          = deviceState.device_type;

    // Dump config.
    json["cfg"]["address"] = ToHex(deviceAddress);
    json["cfg"]["addressCheck"] = addressCheck ? "Y" : "N";
    json["cfg"]["debugMode"] = debugMode ? "Y" : "N";
    json["cfg"]["autoAck"] = autoAck ? "Y" : "N";

    // Serialize unknown commands.
    char sid[8];
    char sval[8];
    for (uint16_t i = 0; i < MAX_UNKNOWN_COMMANDS; i++)
    {
        if (deviceState.ucmd_id[i] == 0) break;

        sniprintf(sid, sizeof(sid), "#%04x", deviceState.ucmd_id[i]);

        if (raw)
        {
            json[sid] = deviceState.ucmd_data[i];
        }
        else
        {
            sniprintf(sval, sizeof(sval), "#%04x", deviceState.ucmd_data[i]);
            json[sid] = sval;
        }
    }

    serializeJson(json, output);
    //serializeJsonPretty(json, Serial);
}

void CTechManager::GetStatsJson(Print& output, EStatsType type)
{
    StatsData* data = nullptr;
    switch (type)
    {
        case(EStatsType::co):
            data = &coStats;
        break;

        case(EStatsType::cwu):
            data = &cwuStats;
        break;

        default:
            data = &extStats;
            break;
    }

    // Update json.
    json.clear();
    
    uint16_t len = data->Count();
    uint16_t readPtr = data->readPtr;
    char buf[8];

    for (uint16_t i = 0; i < len; i++)
    {
        sniprintf(buf, sizeof(buf), "#%04x", data->buffer[readPtr]);
        json.add(buf);
        readPtr = (readPtr + 1) % STATS_DEPTH;
    }

    serializeJson(json, output);
    //serializeJson(json, Serial);
}

void CTechManager::SendCommand(ETechCommand cmd, uint16_t value)
{
    if (txSize >= (MAX_PACKET_SIZE - 4)) return;

    // Store command.
    txBuffer[txSize++] = cmd;
    txBuffer[txSize++] = value;
}


void CTechManager::StoreStats()
{
    firstCo = false;
    firstCwu = false;
    firstExt = false;

    coStats.Append(deviceState.co_temp);
    cwuStats.Append(deviceState.cwu_temp);
    extStats.Append(deviceState.external_temp);
}

uint16_t CTechManager::CRC16Cycle(uint16_t crc, uint8_t byte)
{
    crc ^= byte;
    for (int i = 0; i < 8; i++) {
        if (crc & 0x0001)
            crc = (crc >> 1) ^ 0x8408;
        else
            crc = (crc >> 1);
    }
    return crc;
}

uint16_t CTechManager::ComputeCRC(uint16_t* packet, uint16_t packetLen)
{
    uint16_t i;
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < packetLen; i++)
    {
        // MSB
        crc = CRC16Cycle(crc, (packet[i] & 0xFF00) >> 8);
        // LSB
        crc = CRC16Cycle(crc, (packet[i] & 0xFF));
    }
    return crc;        
}

void CTechManager::ResetTransmitter()
{
    txSize = 0;
    txBuffer[txSize++] = ETechCommand::FRAME_MAGIC;
    txBuffer[txSize++] = ETechDeviceAddress::All;
}

void CTechManager::SendPacket()
{
    if (ioStream != nullptr)
    {
        uint16_t crc = ComputeCRC(txBuffer, txSize);
        
        txBuffer[txSize++] = ETechCommand::CMD_CRC;
        txBuffer[txSize++] = crc;

        for (uint16_t i = 0; i < txSize; i++)
        {
            // MSB first (big-endian on line).
            ioStream->write((char)((txBuffer[i] >> 8) & 0xFF));

            // LSB second.
            ioStream->write((char)(txBuffer[i] & 0xFF));
        }
    }

    // Reset transmitter.
    ResetTransmitter();
}


void CTechManager::ResetReader()
{
    readerState = ReaderState::magic_start;
    readerCounter = 0;
    readerValue = 0;
    readerCmdLen = 0;
    packetSize = 0;
}

bool CTechManager::AppendByte(uint8_t data)
{
    uint16_t crc;

    // Read data value (big-endian to little endian).
    readerValue = (readerCounter == 0) ? data : ((readerValue << 8) & 0xFF00) | (data & 0x00FF);

    // Update counter.
    if ((readerState == ReaderState::magic_start) || (readerState == ReaderState::magic_end))
    {
        readerCounter = 0;
    }
    else
    {
        // Nibble mode.
        readerCounter = (readerCounter + 1) % 2;
        if (readerCounter == 1) return(false);
    };

    switch(readerState)
    {
        case(ReaderState::magic_start): // Look for 0x02.
            if (readerValue == 0x02)
            {
                readerState = ReaderState::magic_end;
            }
            else
            {
                ResetReader();
            }
            break;

        case(ReaderState::magic_end): // Look for 0x26.
            if (readerValue == 0x26)
            {
                readerState = ReaderState::address;
                packet[packetSize++] = 0x0226;
            }
            else
            {
                ResetReader();
            }
            break;

        case(ReaderState::address): // Read address.
            packet[packetSize++] = readerValue;
            readerState = ReaderState::command;
            break;

        case(ReaderState::command): // Read command.
            
            // Check command type.
            switch(readerValue)
            {
                // CRC
                case(ETechCommand::CMD_CRC):
                {
                    readerState = ReaderState::crc;
                    break;
                }

                // Multi-word commands.
                case(ETechCommand::ETH_1788):
                case(ETechCommand::ETH_0400):
                    readerState = ReaderState::data_len;
                    break;

                // Default - single word command.
                default:
                    readerState = ReaderState::data;
                    readerCmdLen = 1;
                    break;
            }

            // Store command id.
            packet[packetSize++] = readerValue;
            break;

        case(ReaderState::data_len):
            packet[packetSize++] = readerValue;

            // Check zero paddings.
            if ((readerValue % 4) != 0)
                readerValue = ((readerValue >> 2) + 1) << 4;

            readerCmdLen = readerValue;

            // Read data.
            readerState = ReaderState::data;
        break;

        case(ReaderState::data):
            packet[packetSize++] = readerValue;
            readerCmdLen--;

            // Next command.
            if (readerCmdLen <= 0)
                readerState = ReaderState::command;
        break;

        case(ReaderState::crc):
            
            // Complete packet.
            packet[packetSize++] = readerValue;

            // Check CRC.
            crc = ComputeCRC(packet, packetSize - 2);
            if (readerValue == crc)
            {
                // Process packet.
                ProcessPacket();
            }

            // Restart reader.
            ResetReader();

            return(true);
        break;

        default:
            ResetReader();
            break;
    }

    return(false);
}

void CTechManager::ProcessPacket()
{
    // Check device address.
    if (addressCheck && (deviceAddress != 0) && (deviceAddress != packet[1])) return;

    // Parse commands.
    uint16_t dataPtr = 2;
    while(dataPtr < packetSize)
    {
        uint16_t cmd_id  = packet[dataPtr++];
        uint16_t cmd_val = packet[dataPtr++];

        // Process command.
        switch(cmd_id)
        {
            // General device status.
            case(ETechCommand::DEVICE_TYPE): // 0x15a7
                deviceState.device_type = cmd_val;
            break;

            case(ETechCommand::DEVICE_TIME): // 0x1620
                deviceState.device_time =  cmd_val;
            break;

            case(ETechCommand::DEVICE_DAY): // 0x1621
                deviceState.device_day =  cmd_val;
            break;

            case(ETechCommand::FUMES_TEMP): // 0x15B7
                deviceState.fumes_temp =  cmd_val;
            break;

            case(ETechCommand::EXTERNAL_TEMP): // 0x1681
                deviceState.external_temp =  cmd_val;

                if (firstExt)
                {
                    firstExt = false;
                    extStats.Append(deviceState.external_temp);
                }

            break;

            case(ETechCommand::CO_TEMP): // 0x157D
                deviceState.co_temp =  cmd_val;

                if (firstCo)
                {
                    firstCo = false;
                    coStats.Append(deviceState.co_temp);
                }

            break;

            case(ETechCommand::CO_MIN_MAX): // 0x169E
                deviceState.co_min_max =  cmd_val;
            break;

            case(ETechCommand::CO_TEMP_SET): // 0x157E
                deviceState.co_temp_set =  cmd_val;
            break;

            case(ETechCommand::CO_TEMP_ADJUSTMENT): // 0x157E
                deviceState.co_temp_adj =  cmd_val;
            break;

            case(ETechCommand::CWU_TEMP): // 0x166E
                deviceState.cwu_temp =  cmd_val;

                if (firstCwu)
                {
                    firstCwu = false;
                    cwuStats.Append(deviceState.cwu_temp);
                }
            break;

            case(ETechCommand::CWU_MIN_MAX): // 0x169F
                deviceState.cwu_min_max =  cmd_val;
            break;

            case(ETechCommand::CWU_TEMP_SET): // 0x1616
                deviceState.cwu_temp_set =  cmd_val;
            break;

            case(ETechCommand::CWU_TEMP_RET): // 0x16C1
                deviceState.cwu_temp_ret =  cmd_val;
            break;

            case(ETechCommand::FUEL_STOCK_LEVEL): // 0x16F1
                deviceState.fuel_stock_level =  cmd_val;
            break;

            case(ETechCommand::FUEL_STOCK_TIME): // 0x16F2
                deviceState.fuel_stock_time =  cmd_val;
            break;

            case(ETechCommand::PUMP_MODE): // 0x15CD
                deviceState.pump_mode =  cmd_val;
            break;

            case(ETechCommand::PUMP_STATE_CO): // 0x1589
                deviceState.pump_state_co =  cmd_val;
            break;

            case(ETechCommand::PUMP_STATE_CWU): // 0x158B
                deviceState.pump_state_cwu =  cmd_val;
            break;

            case(ETechCommand::FAN_STATE): // 0x1588
                deviceState.fan_state =  cmd_val;
            break;

            case(ETechCommand::FAN_SPEED): // 0x159B
                deviceState.fan_speed =  cmd_val;
            break;

            case(ETechCommand::FEEDER_STATE): // 0x1587
                deviceState.feeder_state =  cmd_val;
            break;

            case(ETechCommand::FEEDER_TEMP): // 0x16F8
                deviceState.feeder_temp = cmd_val;
            break;

            // Room controller.
            case(ETechCommand::REG_SET_TEMP_ROOM): // 0x0405
            break;

            case(ETechCommand::REG_SET_TEMP_CO): // 0x01f6
            break;

            case(ETechCommand::REG_SET_TEMP_CWU): // 0x028E
            break;

            case(ETechCommand::REG_TIME): // 0x0298
            break;

            case(ETechCommand::REG_DAY): // 0x0299
            break;

            case(ETechCommand::REG_PUMP_MODE): // 0x0245
            break;

            // Valve controll.
            case(ETechCommand::VALVE_DATA_SET): // 0x16EF
                currentValveSet = cmd_val;
            break;

            case(ETechCommand::VALVE_ADDRESS): // 0x16C2
                deviceState.valveData[currentValveSet].address = cmd_val;
            break;

            case(ETechCommand::VALVE_TEMP_SET): // 0x167F
                deviceState.valveData[currentValveSet].temp_set = cmd_val;
            break;

            case(ETechCommand::VALVE_STATE): // 0x1680
                deviceState.valveData[currentValveSet].state = cmd_val;
            break;

            case(ETechCommand::VALVE_OPEN_LEVEL): // 0x15AC
                deviceState.valveData[currentValveSet].openLevel = cmd_val;
            break;

            case(ETechCommand::VALVE_TYPE): // 0x1624
                deviceState.valveData[currentValveSet].type  = cmd_val;
            break;

            case(ETechCommand::VALVE_TEMP): // 0x1614
                deviceState.valveData[currentValveSet].temp  = cmd_val;
            break;

            case(ETechCommand::VALVE_MIN_MAX): // 0x16C3
                deviceState.valveData[currentValveSet].minMax  = cmd_val;
            break;

            case(ETechCommand::CMD_CRC): // Skip crc.
            break;

            // Multibyte data.
            case(ETechCommand::ETH_1788): //
            case(ETechCommand::ETH_0400): //

                // Skip data.
                if ((cmd_val % 4) != 0)
                    cmd_val = ((cmd_val >> 2) + 1) << 4;
                dataPtr += cmd_val;
            break;

            default: // Dump unknown commands.
                Serial.print("Unknown command. ");
                Serial.print("ID: ");
                Serial.print(cmd_id);
                Serial.print(",   val: ");
                Serial.println(cmd_val);
                UpdateUnknownCommand(cmd_id, cmd_val);
            break;
        }
    }
}

void CTechManager::UpdateUnknownCommand(uint16_t id, uint16_t val)
{
    for (uint16_t i = 0; i < MAX_UNKNOWN_COMMANDS; i++)
    {
        // Update command.
        if (deviceState.ucmd_id[i] == id)
        {
            deviceState.ucmd_data[i] = val;
            break;
        }

        // Command not found (first empty slot), store it.
        if (deviceState.ucmd_id[i] == 0)
        {
            deviceState.ucmd_id[i] = id;
            deviceState.ucmd_data[i] = val;
            break;
        }
    }
}

char* CTechManager::GetTempMeasured(int16_t value)
{
    sniprintf(value_string, sizeof(value_string), "%0.2f", value / 10.0);
    return(value_string);
}

char* CTechManager::GetTempSet(int16_t value)
{
    sniprintf(value_string, sizeof(value_string), "%d", value);
    return(value_string);
}

char* CTechManager::GetTempMin(uint16_t value)
{
    sniprintf(value_string, sizeof(value_string), "%d", value & 0xFF);
    return(value_string);
}

char* CTechManager::GetTempMax(uint16_t value)
{
    sniprintf(value_string, sizeof(value_string), "%d", (value >> 8) & 0xFF);
    return(value_string);
}

char* CTechManager::GetPercentPrecise(uint16_t value)
{
    sniprintf(value_string, sizeof(value_string), "%0.2f", value / 512.0);
    return(value_string);
}
        
char* CTechManager::GetTime(uint16_t value)
{
    sniprintf(value_string, sizeof(value_string), "%02d:%02d", (value >> 8) & 0xFF, (value & 0xFF) );
    return(value_string);
}

char* CTechManager::ToHex(uint16_t value)
{
    sniprintf(value_string, sizeof(value_string), "%02x", value);
    return(value_string);
}
