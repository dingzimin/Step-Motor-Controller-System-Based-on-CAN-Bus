#include "ptl.h"

StreamStartHandler Ptl::OnStreamStart;

bool Ptl::SendData(uint32_t id, uint8_t *data, uint8_t size)
{
    // Magic : 2 , Size : 1 , SeqId : 1 , Crc : 4
    CanTxMsg msg{id, id, CAN_Id_Standard, CAN_RTR_Data, 8, {0}};
    CanRxMsg rmsg;
    uint32_t timeMax = Timer::GetTick() + 500;
    // Prepare data
    CRC_ResetDR();
    uint32_t crc = CRC_CalcBlockCRC((uint32_t *)data, size >> 2);
    if (size & 3)
        crc = CRC_CalcCRC(((uint32_t *)data)[size >> 2] & ~(((int32_t)-1) >> (((~size) & 3) * 8 - 1)));
    srand(Timer::GetTick());
    msg.Data[2] = size;
    msg.Data[3] = rand();
    ((uint32_t *)msg.Data)[1] = crc;
    ((uint16_t *)msg.Data)[0] = ((uint16_t *)msg.Data)[1] ^ ((uint16_t *)msg.Data)[2] ^ ((uint16_t *)msg.Data)[3];
    Can::Transmit(msg);
    // Wait for Ack
    while (true)
    {
        if (Can::Receice(rmsg))
        {
            if (rmsg.StdId == msg.StdId && *((uint64_t *)rmsg.Data) == *((uint64_t *)msg.Data))
                break;
        }
        if (timeMax < Timer::GetTick())
            return false;
    }
    timeMax = Timer::GetTick() + 5000;
    // SendData
    msg.Data[0] = msg.Data[3];
    for (int i = 0; i < size; i += 7)
    {
        memcpy(&msg.Data[1], &data[i], (size - i) < 7 ? (size - i) : 7);
        while (Can::Transmit(msg) == CAN_TxStatus_NoMailBox)
            if (timeMax < Timer::GetTick())
                return false;
        msg.Data[0]++;
    }
    timeMax = Timer::GetTick() + 500;
    while (true)
    {
        if (Can::Receice(rmsg))
        {
            if (rmsg.StdId == msg.StdId && ((uint32_t *)rmsg.Data)[1] == crc)
                break;
        }
        if (timeMax < Timer::GetTick())
            return false;
    }
    return true;
}

bool Ptl::ReceiveData(uint32_t id, uint8_t *buffer, uint8_t size, uint8_t sseq, uint32_t ccrc)
{
    CanRxMsg msg{id, id, CAN_Id_Standard, CAN_RTR_Remote, 0, {0}};
    uint32_t timeMax = Timer::GetTick() + 5000;
    uint32_t progress = (1 << (size / 7 + 1)) - 1;
    while (progress)
    {
        if (!Can::Receice(msg) || msg.StdId != id || msg.RTR != CAN_RTR_Data)
            continue;
        uint8_t seq = msg.Data[0];
        uint8_t pos = seq - sseq;
        memcpy(&buffer[pos * 7], &msg.Data[1], (size - pos) < 7 ? (size - pos) : 7);
        *BitBand::Ram(&progress, pos) = 0;
        if (Timer::GetTick() > timeMax)
            return false;
    }
    CRC_ResetDR();
    uint32_t crc = CRC_CalcBlockCRC((uint32_t *)buffer, size >> 2);
    if (size & 3)
        crc = CRC_CalcCRC(((uint32_t *)buffer)[size >> 2] & ~(((int32_t)-1) >> (((~size) & 3) * 8 - 1)));
    Can::Transmit(id, CAN_Id_Standard, CAN_RTR_Data, (uint8_t *)&crc, 4);
	return crc == ccrc;
}
