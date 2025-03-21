#include "CTechManager.h"

CTechManager::CTechManager(uint16_t addr)
{
    deviceAddress = addr;
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
}

float CTechManager::GetState(uint16_t value, int valve)
{
    int data = 0;
    switch(value)
    {
        case(ETechCommand::VALVE_STATE):
            data = deviceState.valveData[valve].state;
            break;

        case(ETechCommand::VALVE_TYPE):
            data = deviceState.valveData[valve].type;
            break;
        
        case(ETechCommand::VALVE_OPEN_LEVEL):
            data = deviceState.valveData[valve].openLevel;
            break;

        case(ETechCommand::VALVE_TEMP_SET):
            data = deviceState.valveData[valve].temp_set;
            break;

        case(ETechCommand::VALVE_TEMP):
            data = deviceState.valveData[valve].temp / 10;
            break;

        case(ETechCommand::VALVE_PUMP_STATE):
            data = deviceState.valveData[valve].pump_state;
            break;
    }
    return data;
}

float CTechManager::GetState(uint16_t value)
{
    float data = 0;
    switch(value)
    {
        case(ETechCommand::DEVICE_TIME):
            data = deviceState.device_time;
            break;

        case(ETechCommand::EXTERNAL_TEMP):
            if(deviceState.external_temp >= 32768)
            {
                data = (deviceState.external_temp - 65536) / 10;
            } else {
                data = deviceState.external_temp / 10;
            }
            break;
        
        case(ETechCommand::DEVICE_STATE):
            data = deviceState.device_state;
            break;

        case(ETechCommand::CO_TEMP):
            data = deviceState.co_temp / 10;
            break;

        case(ETechCommand::CO_TEMP_RET):
            data = deviceState.co_temp_ret / 10;
            break;

        case(ETechCommand::CWU_TEMP):
            data = deviceState.cwu_temp /10;
            break;
        
        case(ETechCommand::PUMP_STATE_CO):
            data = deviceState.pump_state_co;
            break;

        case(ETechCommand::PUMP_STATE_CWU):
            data = deviceState.pump_state_cwu;
            break;

        case(ETechCommand::PUMP_MODE):
            data = deviceState.pump_mode;
            break;
        
        case(ETechCommand::CWU_TEMP_SET):
            data = deviceState.cwu_temp_set;
            break;
    }
    return data;
}

void CTechManager::SendCommand(ETechCommand cmd, uint16_t value)
{
    /*
    Serial.print("Sending command:   ");
    Serial.print(cmd);
    Serial.print("   value:   ");
    Serial.println(value);
    */

    if (txSize >= (MAX_PACKET_SIZE - 4)) return;

    // Store command.
    txBuffer[txSize++] = cmd;
    txBuffer[txSize++] = value;
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

            case(ETechCommand::DEVICE_MODEL): // 0x16ff
                deviceState.device_model = cmd_val;
            break;

            case(ETechCommand::DEVICE_TIME): // 0x1620
                deviceState.device_time =  cmd_val;
            break;

            case(ETechCommand::DEVICE_DAY): // 0x1621
                deviceState.device_day =  cmd_val;
            break;

            case(ETechCommand::DEVICE_STATE): // 0x157C
                deviceState.device_state =  cmd_val;
            break;

            case(ETechCommand::FUMES_TEMP): // 0x15B7
                deviceState.fumes_temp =  cmd_val;
            break;

            case(ETechCommand::EXTERNAL_TEMP): // 0x1681
                deviceState.external_temp =  cmd_val;
            break;

            case(ETechCommand::CO_TEMP): // 0x157D
                deviceState.co_temp =  cmd_val;
            break;

            case(ETechCommand::CO_TEMP_RET): // 0x16C1
                deviceState.co_temp_ret =  cmd_val;
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
            break;

            case(ETechCommand::CWU_MIN_MAX): // 0x169F
                deviceState.cwu_min_max =  cmd_val;
            break;

            case(ETechCommand::CWU_TEMP_SET): // 0x1616
                deviceState.cwu_temp_set =  cmd_val;
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

            case(ETechCommand::VALVE_PUMP_STATE): // 0x16B9
                deviceState.valveData[currentValveSet].pump_state = cmd_val;
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
            /*
                Serial.print("Unknown command. ");
                Serial.print("ID: ");
                Serial.print(cmd_id);
                Serial.print(",   val: ");
                Serial.println(cmd_val);
            */
            break;
        }
    }
}
