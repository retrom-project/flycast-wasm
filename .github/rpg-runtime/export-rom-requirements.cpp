// Export the exact table compiled into this Flycast build, including virtual and
// optional ROM semantics. No game bytes or frontend filename heuristics.
#include "hw/naomi/naomi_roms.cpp"
#include <cstdio>
#include <cstring>

static void string(const char* value) {
    if (!value) { std::fputs("null", stdout); return; }
    std::putchar('"');
    for (const unsigned char* c = reinterpret_cast<const unsigned char*>(value); *c; ++c) {
        if (*c == '"' || *c == '\\') { std::putchar('\\'); std::putchar(*c); }
        else if (*c < 32) std::printf("\\u%04x", *c);
        else std::putchar(*c);
    }
    std::putchar('"');
}

int main() {
    std::fputs("[", stdout);
    bool firstGame = true;
    for (const Game* game = Games; game->name; ++game) {
        if (!firstGame) std::putchar(',');
        firstGame = false;
        std::fputs("{\"name\":", stdout); string(game->name);
        std::fputs(",\"parent\":", stdout); string(game->parent_name);
        const bool systemsp = game->bios && std::strcmp(game->bios, "segasp") == 0;
        const char* platform = game->cart_type == AW ? "atomiswave" : systemsp ? "systemsp" :
            (game->bios && std::strcmp(game->bios, "naomi2") == 0 ? "naomi2" : "naomi");
        std::fputs(",\"platform\":", stdout); string(platform);
        std::fputs(",\"mediaType\":", stdout); string(game->cart_type == GD ? "GDROM" : systemsp && game->gdrom_name ? "COMPACT_FLASH" : "CARTRIDGE");
        std::fputs(",\"disc\":", stdout); string(game->gdrom_name);
        std::fputs(",\"files\":[", stdout);
        bool firstFile = true;
        for (const auto& blob : game->blobs) {
            if (!blob.filename) break;
            if (blob.blob_type == Copy) continue; // Copies are internal cartridge memory operations.
            if (!firstFile) std::putchar(',');
            firstFile = false;
            std::fputs("{\"name\":", stdout); string(blob.filename);
            std::printf(",\"sizeBytes\":%u,\"crc32\":", blob.length);
            if (blob.crc) std::printf("\"%08x\"", blob.crc); else std::fputs("null", stdout);
            std::printf(",\"optional\":%s}", blob.blob_type == Eeprom ? "true" : "false");
        }
        std::fputs("]}", stdout);
    }
    std::fputs("]\n", stdout);
    return std::ferror(stdout) ? 1 : 0;
}
