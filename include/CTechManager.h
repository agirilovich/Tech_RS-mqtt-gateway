#include "Arduino.h"

class CTechManager
{
    private:
        static const uint16_t MAX_PACKET_SIZE = 500;
        static const uint16_t MAX_UNKNOWN_COMMANDS = 100;
        static const uint16_t STATS_DEPTH = 1024;

    public:

        struct ValveData
        {
            uint16_t dataSet;
            uint16_t address;
            uint16_t temp_set;
            uint16_t state;
            uint16_t openLevel;
            uint16_t type;
            uint16_t temp;
            uint16_t minMax;
            uint16_t pump_state;
        };

        struct DeviceData
        {
            uint16_t device_type;
            uint16_t device_time;
            uint16_t device_day;
            uint16_t device_model;
            uint16_t device_state;

            uint16_t fumes_temp;
            uint16_t external_temp;
            
            uint16_t co_temp;
            uint16_t co_min_max;
            uint16_t co_temp_set;
            uint16_t co_temp_adj;
            
            uint16_t cwu_temp;
            uint16_t cwu_min_max;
            uint16_t cwu_temp_set;
            uint16_t cwu_temp_ret;

            uint16_t fuel_stock_level;
            uint16_t fuel_stock_time;

            uint16_t pump_mode;
            uint16_t pump_state_co;
            uint16_t pump_state_cwu;

            uint16_t fan_state;
            uint16_t fan_speed;

            uint16_t feeder_state;
            uint16_t feeder_temp;

            ValveData valveData[4] = { {0}, {0}, {0}, {0}};

            uint16_t ucmd_id[MAX_UNKNOWN_COMMANDS] = {0};
            uint16_t ucmd_data[MAX_UNKNOWN_COMMANDS] = {0};
        };

    private:

        uint16_t ComputeCRC(uint16_t* packet, uint16_t packetLen);
        uint16_t CRC16Cycle(uint16_t crc, uint8_t byte);

        enum ReaderState : uint8_t
        {
            magic_start = 0,
            magic_end,
            address,
            command,
            data_len,
            data,
            crc
        };

        // Frame reader.
        uint8_t readerCounter = 0;
        uint16_t readerValue = 0;
        int16_t readerCmdLen = 0;
        
        uint16_t packetSize = 0;
        uint16_t packet[MAX_PACKET_SIZE] = {0};

        ReaderState readerState = ReaderState::magic_start;

        // Transmitter.
        uint16_t txBuffer[MAX_PACKET_SIZE] = {0};
        uint16_t txSize = 0;

        // Device state.
        DeviceData deviceState;

        // Internal utils.
        void ResetReader();
        bool AppendByte(uint8_t data);
        void ProcessPacket();

        // Serialization helpers.
        uint8_t currentValveSet = 0;
        char value_string[32];
        
        char* GetTempMeasured(int16_t value);
        char* GetTempSet(int16_t value);
        char* GetTempMin(uint16_t value);
        char* GetTempMax(uint16_t value);
        char* GetPercentPrecise(uint16_t value);
        char* GetTime(uint16_t value);

        void UpdateUnknownCommand(uint16_t id, uint16_t val);

        Stream* ioStream = nullptr;

        uint16_t requestStamp = 0;

        void ResetTransmitter();
        void SendPacket();

        // Config.
        uint16_t deviceAddress = 0xFFF8;

        bool debugMode = false;
        bool autoAck = true;
        bool addressCheck = true;

        // Stats.
        struct StatsData
        {
            uint16_t writePtr = 0;
            uint16_t readPtr = 0;
            uint16_t buffer[STATS_DEPTH] = {0};

            void Append(uint16_t data)
            {
                buffer[writePtr] = data;
                writePtr = (writePtr + 1) % STATS_DEPTH;

                if (writePtr == readPtr) 
                    readPtr = (readPtr + 1) % STATS_DEPTH;
            };

            uint16_t Count()
            {
                if (readPtr > writePtr)
                    return((writePtr + STATS_DEPTH) - readPtr);
                
                return(writePtr - readPtr);
            }
        };

        StatsData coStats;
        StatsData cwuStats;
        StatsData extStats;
        
        ulong statsStamp = 0;
        ulong statsDelay = 0;

        bool firstCo = true;
        bool firstCwu = true;
        bool firstExt = true;
        void StoreStats();

    public :

        enum ETechDeviceAddress : uint16_t
        {
            All = 0x0000,
            Ethernet = 0xFFF4,
            GSM = 0xFFF8,
            Room = 0xFFFA
        };

        enum ETechCommand : uint16_t
        {
            FRAME_MAGIC = 0x226,
            CMD_CRC = 0x0218,

            ETH_1788 = 0x1788,
            ETH_0400 = 0x0400,

            // General.
            DEVICE_TYPE = 0x15a7,
            DEVICE_TIME = 0x1620,
            DEVICE_DAY = 0x1621,
            DEVICE_MODEL = 0x16FF,
            DEVICE_STATE = 0x01F4,

            FUMES_TEMP = 0x15B7,
            EXTERNAL_TEMP = 0x1681,
            
            CO_TEMP = 0x157D,
            CO_TEMP_RET = 0x156D,
            CO_MIN_MAX = 0x169E,
            CO_TEMP_SET = 0x157E,
            CO_TEMP_ADJUSTMENT = 0x1684, // *** new value.
            
            CWU_TEMP = 0x166E,
            CWU_MIN_MAX = 0x169F,
            CWU_TEMP_SET = 0x1616,
            CWU_TEMP_RET = 0x16C1,
            
            FUEL_STOCK_LEVEL = 0x16F1,
            FUEL_STOCK_TIME = 0x16F2,
            
            PUMP_MODE = 0x15CD,
            PUMP_STATE_CO = 0x1589,
            PUMP_STATE_CWU = 0x158B,
            
            FAN_STATE = 0x1588,
            FAN_SPEED = 0x159B,
            
            FEEDER_STATE = 0x1587,
            FEEDER_TEMP = 0x16F8,
            
            // Room controller.
            REG_SET_TEMP_ROOM = 0x0405,
            REG_SET_TEMP_CO = 0x01f6,
            REG_SET_TEMP_CWU = 0x028E,
            REG_TIME = 0x0298,
            REG_DAY = 0x0299,
            REG_PUMP_MODE = 0x0245,
            REG_TEMP_SET = 0x0312,
            REG_TEMP = 0x0311,

            // Valves.
            VALVE_DATA_SET = 0x16EF,
            VALVE_ADDRESS = 0x16C2,
            VALVE_TEMP_SET = 0x167F,
            VALVE_STATE = 0x1680,
            VALVE_OPEN_LEVEL = 0x15AC,
            VALVE_TYPE = 0x1624,
            VALVE_TEMP = 0x1614,
            VALVE_MIN_MAX = 0x16C3,
            VALVE_PUMP_STATE = 0x16B9,

            // Command set.
            SET_PUMP_MODE = 0x245,

            // Commands sniffed from ST-65 GSM module.
            SET_TEMP_CO = 0x01f6,
            SET_TEMP_CWU = 0x028E,

            CO_VALVE_ADJ = 0x170A // **** valve adjustment.

        };

        // API
        CTechManager(uint16_t deviceAddress = ETechDeviceAddress::GSM);

        void SetStream(Stream* stream) { ioStream = stream; };
        
        void Update();

        void SendCommand(ETechCommand cmd, uint16_t value);

        float GetState(uint16_t value);
        float GetState(uint16_t value, int valve);

        // Dev.
        void SetDebug(bool val) { debugMode = val; };
        void SetAutoAck(bool val) { autoAck = val; };
        void SetAddress(uint16_t address) { deviceAddress = address; };
        void SetAddressCheck(bool val) { addressCheck = val; };
        void SetStatsDelay(uint16_t val) { statsDelay = ((ulong)val) * 1000; };
};