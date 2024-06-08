#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/aes.h>
#include <sstream>

namespace AES {
    std::string encrypt(const std::string& content, const std::string& key, const std::string& iv, const EVP_CIPHER* algo) {
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        int len = 0, cipher_len = 0;
        u_char enc_buffer[128];

        EVP_EncryptInit_ex(ctx, algo, NULL, (u_char*)key.c_str(), (u_char*)iv.c_str());
        EVP_EncryptUpdate(ctx, enc_buffer, &len, (u_char*)content.c_str(), content.size());
        cipher_len = len;

        EVP_EncryptFinal_ex(ctx, enc_buffer + len, &len);
        cipher_len += len;

        EVP_CIPHER_CTX_free(ctx);
        return std::string((char*)enc_buffer, cipher_len);
    }

    std::string decrypt(const std::string& cipher, const std::string& key, const std::string& iv, const EVP_CIPHER* algo) {
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        int len = 0, plaintext_len = 0, ret;
        u_char dec_buffer[512];

        EVP_DecryptInit_ex(ctx, algo, NULL, (u_char*)key.c_str(), (u_char*)iv.c_str());
        EVP_DecryptUpdate(ctx, dec_buffer, &len, (u_char*)cipher.c_str(), cipher.size());
        plaintext_len = len;
        
        ret = EVP_DecryptFinal_ex(ctx, dec_buffer + len, &len);
        plaintext_len += len;

        EVP_CIPHER_CTX_free(ctx);
        return std::string((char*)dec_buffer, plaintext_len);
    }
}

namespace crypto {
    /*
    std::string base64_encode(const std::string& input) {
        const auto base64_memory = BIO_new(BIO_s_mem());
        auto base64 = BIO_new(BIO_f_base64());

        base64 = BIO_push(base64, base64_memory);

        BIO_write(base64, input.c_str(), input.length());
        BIO_flush(base64);
        
        BUF_MEM* buffer_memory{};
        BIO_get_mem_ptr(base64, &buffer_memory);
        auto base64_encoded = std::string(buffer_memory->data, buffer_memory->length - 1);

        BIO_free_all(base64);
        return base64_encoded;
    }

    std::string base64_decode(const std::string& input) {
        auto base64_memory = BIO_new_mem_buf(input.c_str(), input.length());
        const auto base64 = BIO_new(BIO_f_base64());

        BIO_set_flags(base64, BIO_FLAGS_BASE64_NO_NL);
        BIO_push(base64, base64_memory);
        
        const int maxlen = input.length() / 4 * 3 + 1;
        char buffer[maxlen];
        const int len = BIO_read(base64, buffer, maxlen);

        BIO_free_all(base64);
        return std::string(buffer, len);
    } */

    std::string base64_encode(const std::string& binary) {
        auto b64 = BIO_new(BIO_f_base64());
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO* sink = BIO_new(BIO_s_mem());
        BIO_push(b64, sink);
        BIO_write(b64, binary.c_str(), binary.size());
        BIO_flush(b64);

        const char* encoded;
        const long len = BIO_get_mem_data(sink, &encoded);
        return std::string(encoded, len);
    }

    std::string base64_decode(const std::string& encoded) {
        auto b64 = BIO_new(BIO_f_base64());
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO* source = BIO_new_mem_buf(encoded.c_str(), -1);
        BIO_push(b64, source);

        const int maxlen = encoded.size() / 4 * 3 + 1;
        std::vector<unsigned char> decoded(maxlen);
        const int len = BIO_read(b64, decoded.data(), maxlen);
        decoded.resize(len);
        return std::string(decoded.begin(), decoded.end());
    }

    std::string sha256(const std::string& input) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        auto data = (const unsigned char*)input.c_str();
        SHA256(data, input.size(), hash);
        std::stringstream ss;

        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }

        return ss.str();
    }

    std::string sha3_256(const std::string& input) {
        u_int hash_len = SHA256_DIGEST_LENGTH;
        u_char* hash = (u_char*)OPENSSL_malloc(hash_len);
        EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

        EVP_DigestInit_ex(mdctx, EVP_sha3_512(), NULL);
        EVP_DigestUpdate(mdctx, input.c_str(), input.size());
        EVP_DigestFinal_ex(mdctx, hash, &hash_len);

        EVP_MD_CTX_destroy(mdctx);
        std::stringstream ss;
        
        for (int i = 0; i < hash_len; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }

        OPENSSL_free(hash);
        return ss.str();
    }

    std::string sha512(const std::string& input) {
        unsigned char hash[SHA512_DIGEST_LENGTH];
        auto data = (const unsigned char*)input.c_str();
        SHA512(data, input.size(), hash);
        std::stringstream ss;

        for (int i = 0; i < SHA512_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }

        return ss.str();
    }

    int generatebytes(uint64_t rl) {
        (*rbx_checkany)(rl, 1);
        int count = rbx_tointeger(rl, 1);
        unsigned char byte_buffer[count];
        if (RAND_bytes(byte_buffer, count) < 0) {
            (*rbx_error)("Failed to generate bytes.");
        }

        rbx_pushstring(rl, base64_encode(std::string((char*)byte_buffer, count)));
        return 1;
    }

    int generatekey(uint64_t rl) {
        unsigned char byte_buffer[32];
        if (RAND_bytes(byte_buffer, 32) < 0) {
            (*rbx_error)("Failed to generate bytes.");
        }

        rbx_pushstring(rl, base64_encode(std::string((char*)byte_buffer, 32)));
        return 1;
    }

    const EVP_CIPHER* resolve_algorithm(const std::string& mode) {
        if (mode == "CBC") {
            return EVP_aes_256_cbc();
        } else if (mode == "ECB") {
            return EVP_aes_256_ecb();
        } else if (mode == "CTR") {
            return EVP_aes_256_ctr();
        } else if (mode == "CFB") {
            return EVP_aes_256_cfb();
        } else if (mode == "OFB") {
            return EVP_aes_256_ofb();
        } else if (mode == "GCM") {
            return EVP_aes_256_gcm();
        }

        return NULL;
    }

    int decrypt(uint64_t rl) {
        (*rbx_checkany)(rl, 4);
        if (rbx_gettype(rl, 1) != LUA_TSTRING || rbx_gettype(rl, 2) != LUA_TSTRING) {
            (*rbx_error)("Invalid arguments for crypt.decrypt");
            return 0;
        }
        
        if (rbx_gettype(rl, 3) != LUA_TSTRING || rbx_gettype(rl, 4) != LUA_TSTRING) {
            (*rbx_error)("Invalid arguments for crypt.decrypt");
            return 0;
        }

        size_t data_len;
        std::string data_key(base64_decode(rbx_tolstring(rl, 2)));
        std::string data(rbx_tolstring(rl, 1, &data_len), data_len);
        if (data_key.size() * 8 != 256) {
            (*rbx_error)("Invalid key for crypt.decrypt");
            return 0;
        }

        std::string iv(base64_decode(rbx_tolstring(rl, 3)));
        if (iv.size() * 8 != 256) {
            (*rbx_error)("Invalid iv for crypt.decrypt");
        }

        std::string mode = rbx_tolstring(rl, 4);
        auto algo = resolve_algorithm(mode);

        if (!algo) (*rbx_error)("Unknown algorithm");
        std::string decoded = AES::decrypt(data, data_key, iv, algo);
        rbx_pushstring(rl, decoded);
        return 1;
    }

    int encrypt(uint64_t rl) {
        if (rbx_gettop(rl) < 2) {
            (*rbx_error)("Invalid arguments for crypt.encrypt");
            return 0;
        }

        if (rbx_gettype(rl, 1) != LUA_TSTRING || rbx_gettype(rl, 2) != LUA_TSTRING) {
            (*rbx_error)("Invalid arguments for crypt.encrypt");
            return 0;
        }

        size_t data_len;
        std::string data_key(base64_decode(rbx_tolstring(rl, 2)));
        std::string data(rbx_tolstring(rl, 1, &data_len), data_len);

        if (data_key.size() * 8 != 256) {
            (*rbx_error)("Invalid key for crypt.encrypt");
            return 0;
        }

        size_t iv_len = 0;
        std::string iv = "";
        bool generated = false;

        if (rbx_gettype(rl, 3) == LUA_TSTRING) {
            iv = base64_decode(std::string(rbx_tolstring(rl, 3, &iv_len), iv_len));

            if (iv.size() != 32) {
                (*rbx_error)("Invalid IV provided for crypt.encrypt");
                return 0;
            }
        }

        if (!iv.size()) {
            unsigned char iv_buffer[32];
            RAND_bytes(iv_buffer, 32);

            iv = std::string((char*)iv_buffer, 32);
            generated = true;
        }

        std::string mode = "CBC";
        if (rbx_gettype(rl, 4) == LUA_TSTRING) {
            mode = rbx_tolstring(rl, 4);
        }

        auto algo = resolve_algorithm(mode);
        if (!algo) (*rbx_error)("Unknown algorithm");
        std::string encrypted = AES::encrypt(data, data_key, iv, algo);
        rbx_pushstring(rl, encrypted);

        if (generated) {
            rbx_pushstring(rl, base64_encode(iv));
            return 2;
        }

        return 1;
    }

    int hash(uint64_t rl) {
        (*rbx_checkany)(rl, 2);
        if (rbx_gettype(rl, 1) != LUA_TSTRING || rbx_gettype(rl, 2) != LUA_TSTRING) {
            (*rbx_error)("Invalid arguments for crypt.hash");
        }

        size_t data_len = 0;
        std::string algo(rbx_tolstring(rl, 2));
        std::string data(rbx_tolstring(rl, 1, &data_len), data_len);

        size_t hash_len;
        u_char* hash;

        if (algo == "sha256") {
            hash_len = SHA256_DIGEST_LENGTH;
            hash = new u_char[hash_len];
            SHA256((u_char*)data.c_str(), data.size(), hash);
        } else if (algo == "sha512") {
            hash_len = SHA512_DIGEST_LENGTH;
            hash = new u_char[hash_len];
            SHA512((u_char*)data.c_str(), data.size(), hash);
        } else if (algo == "sha384") {
            hash_len = SHA384_DIGEST_LENGTH;
            hash = new u_char[hash_len];
            SHA384((u_char*)data.c_str(), data.size(), hash);
        } else if (algo == "sha1") {
            hash_len = 20;
            hash = new u_char[hash_len];
            SHA1((u_char*)data.c_str(), data.size(), hash);
        } else if (algo == "sha3-224") {
            u_int hash_len = SHA224_DIGEST_LENGTH;
            u_char* hash = (u_char*)OPENSSL_malloc(hash_len);
            EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
            
            EVP_DigestInit_ex(mdctx, EVP_sha3_224(), NULL);
            EVP_DigestUpdate(mdctx, data.c_str(), data.size());
            EVP_DigestFinal_ex(mdctx, hash, &hash_len);

            EVP_MD_CTX_destroy(mdctx);
            std::stringstream ss;
            
            for (int i = 0; i < hash_len; i++) {
                ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
            }

            OPENSSL_free(hash);
            rbx_pushstring(rl, ss.str()); return 1;
        } else if (algo == "sha3-256") {
            u_int hash_len = SHA256_DIGEST_LENGTH;
            u_char* hash = (u_char*)OPENSSL_malloc(hash_len);
            EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
            
            EVP_DigestInit_ex(mdctx, EVP_sha3_256(), NULL);
            EVP_DigestUpdate(mdctx, data.c_str(), data.size());
            EVP_DigestFinal_ex(mdctx, hash, &hash_len);

            EVP_MD_CTX_destroy(mdctx);
            std::stringstream ss;
            
            for (int i = 0; i < hash_len; i++) {
                ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
            }

            OPENSSL_free(hash);
            rbx_pushstring(rl, ss.str()); return 1;
        } else if (algo == "sha3-512") {
            u_int hash_len = SHA512_DIGEST_LENGTH;
            u_char* hash = (u_char*)OPENSSL_malloc(hash_len);
            EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
            
            EVP_DigestInit_ex(mdctx, EVP_sha3_512(), NULL);
            EVP_DigestUpdate(mdctx, data.c_str(), data.size());
            EVP_DigestFinal_ex(mdctx, hash, &hash_len);

            EVP_MD_CTX_destroy(mdctx);
            std::stringstream ss;
            
            for (int i = 0; i < hash_len; i++) {
                ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
            }

            OPENSSL_free(hash);
            rbx_pushstring(rl, ss.str()); return 1;
        } else if (algo == "md5") {
            u_int hash_len = EVP_MAX_MD_SIZE;
            u_char* hash = (u_char*)OPENSSL_malloc(hash_len);
            EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
            
            EVP_DigestInit_ex2(mdctx, EVP_md5(), NULL);
            EVP_DigestUpdate(mdctx, data.c_str(), data.size());
            EVP_DigestFinal_ex(mdctx, hash, &hash_len);

            EVP_MD_CTX_destroy(mdctx);
            std::stringstream ss;
            
            for (int i = 0; i < hash_len; i++) {
                ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
            }

            OPENSSL_free(hash);
            rbx_pushstring(rl, ss.str()); return 1;
        } else { rbx_pushstring(rl, "unknown algorithm"); return 1; }

        std::stringstream ss;
        for (int i = 0; i < hash_len; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        
        rbx_pushstring(rl, ss.str());
        return 1;
    }
}