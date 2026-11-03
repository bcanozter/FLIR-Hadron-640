#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

typedef struct {
  // ..... Boson Package (Before bit stuffing) .....
  unsigned char aux_boson_package[768];
  unsigned short aux_boson_package_len;

  // ..... Boson Package (After bit stuffing) .....
  unsigned char boson_stuffed_package[1544];
  unsigned short boson_stuffed_package_len;

  // ..... Received Boson requested data .....
  unsigned char BosonData[768];
  unsigned short BosonData_count;
  unsigned char sequence;
} BosonContext;

uint16_t CalcBlockCRC16(size_t bufferlen, const unsigned char *buffer);
void Boson_Build_FSLP(BosonContext *ctx, uint32_t function,
                      const unsigned char *Data, size_t DataCount);
void Boson_BitStuffing(BosonContext *ctx);
void Boson_BitUnstuffing(BosonContext *ctx);

int Boson_Check_Received_Frame(BosonContext *ctx, const unsigned char *pkg,
                               size_t pkg_len);
void Boson_Status_Error_Codes(int code);

// rawBoson README examples.
namespace boson::cmd {
constexpr uint32_t kBosonSerialNumber = 0x00050002;
constexpr uint32_t kBosonFLATField = 0x00050007; // flat field correction
// data 00 00 00 01 = BlackHot, 00 00 00 00 = WhiteHot
constexpr uint32_t kBosonOutput = 0x000B0003;
} // namespace boson::cmd
