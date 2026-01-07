// Stub implementations for unused algorithms
// This file provides minimal implementations to satisfy linker requirements
// when building a SHA-256d-only version of ccminer

#include "miner.h"
#include <string.h>

// Stub implementations for all unused scanhash_* functions
int scanhash_cryptonight(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done, int variant) { return 0; }
int scanhash_whirl(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_keccak256(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_groestlcoin(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_allium(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_blake256(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done, int8_t blakerounds) { return 0; }
int scanhash_blake2b(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_blake2s(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_lbry(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_luffa(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_lyra2(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_lyra2v2(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_lyra2v3(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_lyra2Z(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_myriad(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_neoscrypt(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_jha(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_fugue256(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_hmq17(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_hsr(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_x11(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_x12(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_x13(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_x14(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_x15(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_x16r(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_x16s(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_x17(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_zr5(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_vanilla(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done, int8_t blakerounds) { return 0; }
int scanhash_veltor(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_wildkeccak(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_jackpot(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_nist5(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_pentablake(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_phi(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_phi2(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_polytimos(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_quark(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_qubit(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_scrypt(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done, unsigned char *scratchbuf, struct timeval *tv_start, struct timeval *tv_end) { return 0; }
int scanhash_scrypt_jane(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done, unsigned char *scratchbuf, struct timeval *tv_start, struct timeval *tv_end) { return 0; }
int scanhash_sha256t(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_sha256q(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_sia(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_sib(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_skeincoin(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_skein2(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_skunk(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_sonoa(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_s3(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_timetravel(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_tribus(int thr_id, struct work *work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_bitcore(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_x11evo(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_bmw(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_bastion(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_c11(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_cryptolight(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done, int variant) { return 0; }
int scanhash_deep(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_decred(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_equihash(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_exosis(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }
int scanhash_fresh(int thr_id, struct work* work, uint32_t max_nonce, unsigned long *hashes_done) { return 0; }

// Stub hash functions
void allium_hash(void *output, const void *input) {}
void bastionhash(void *output, const void *input) {}
void blake256hash(void *output, const void *input, int8_t rounds) {}
void blake2b_hash(void *output, const void *input) {}
void blake2s_hash(void *output, const void *input) {}
void bmw_hash(void *output, const void *input) {}
void c11hash(void *output, const void *input) {}
void cryptolight_hash(void* output, const void* input) {}
void cryptonight_hash(void* output, const void* input) {}
void decred_hash(void *output, const void *input) {}
void deephash(void *output, const void *input) {}
void fresh_hash(void *output, const void *input) {}
void fugue256_hash(void *output, const void *input) {}
void fugue256_hash(unsigned char* output, const unsigned char* input, int len) {}
void groestlhash(void *output, const void *input) {}
void heavycoin_hash(unsigned char* output, const unsigned char* input, int len) {}
void bastionhash(void* output, const unsigned char* input) {}
void hmq17hash(void *output, const void *input) {}
void monero_hash(void* output, const void* input) {}
void hsr_hash(void *output, const void *input) {}
void jha_hash(void *output, const void *input) {}
void keccak256_hash(void *output, const void *input) {}
void lbry_hash(void *output, const void *input) {}
void luffa_hash(void *output, const void *input) {}
void lyra2re_hash(void *output, const void *input) {}
void lyra2v2_hash(void *output, const void *input) {}
void lyra2v3_hash(void *output, const void *input) {}
void lyra2Z_hash(void *output, const void *input) {}
void myriadhash(void *output, const void *input) {}
void neoscrypt(uchar *output, const uchar *input, uint32_t profile) {}
void nist5hash(void *output, const void *input) {}
void pentablakehash(void *output, const void *input) {}
void phi2_hash(void *output, const void *input) {}
void polytimos_hash(void *output, const void *input) {}
void quarkhash(void *output, const void *input) {}
void qubithash(void *output, const void *input) {}
void scrypthash(void* output, const void* input) {}
void scryptjane_hash(void* output, const void* input) {}
void sha256t_hash(void *output, const void *input) {}
void sha256q_hash(void *output, const void *input) {}
void sia_blake2b_hash(void *output, const void *input) {}
void sibhash(void *output, const void *input) {}
void skeincoinhash(void *output, const void *input) {}
void skein2hash(void *output, const void *input) {}
void skunk_hash(void *output, const void *input) {}
void stellite_hash(void *output, const void *input) {}
void s3hash(void *output, const void *input) {}
void timetravel_hash(void *output, const void *input) {}
void bitcore_hash(void *output, const void *input) {}
void exosis_hash(void *output, const void *input) {}
void tribus_hash(void *output, const void *input) {}
void veltorhash(void *output, const void *input) {}
void wcoinhash(void *output, const void *input) {}
void x11evo_hash(void *output, const void *input) {}
void x11hash(void *output, const void *input) {}
void x12hash(void *output, const void *input) {}
void x13hash(void *output, const void *input) {}
void x14hash(void *output, const void *input) {}
void x15hash(void *output, const void *input) {}
void x16r_hash(void *output, const void *input) {}
void x16s_hash(void *output, const void *input) {}
void x17hash(void *output, const void *input) {}
void zr5hash(void *output, const void *input) {}

// Stub free functions
void free_allium(int thr_id) {}
void free_bastion(int thr_id) {}
void free_bitcore(int thr_id) {}
void free_blake256(int thr_id) {}
void free_blake2b(int thr_id) {}
void free_blake2s(int thr_id) {}
void free_bmw(int thr_id) {}
void free_c11(int thr_id) {}
void free_cryptolight(int thr_id) {}
void free_cryptonight(int thr_id) {}
void free_decred(int thr_id) {}
void free_deep(int thr_id) {}
void free_equihash(int thr_id) {}
void free_exosis(int thr_id) {}
void free_keccak256(int thr_id) {}
void free_fresh(int thr_id) {}
void free_fugue256(int thr_id) {}
void free_groestlcoin(int thr_id) {}
void free_hmq17(int thr_id) {}
void free_hsr(int thr_id) {}
void free_jackpot(int thr_id) {}
void free_jha(int thr_id) {}
void free_lbry(int thr_id) {}
void free_luffa(int thr_id) {}
void free_lyra2(int thr_id) {}
void free_lyra2v2(int thr_id) {}
void free_lyra2v3(int thr_id) {}
void free_lyra2Z(int thr_id) {}
void free_myriad(int thr_id) {}
void free_neoscrypt(int thr_id) {}
void free_nist5(int thr_id) {}
void free_pentablake(int thr_id) {}
void free_phi(int thr_id) {}
void free_phi2(int thr_id) {}
void free_polytimos(int thr_id) {}
void free_quark(int thr_id) {}
void free_qubit(int thr_id) {}
void free_skeincoin(int thr_id) {}
void free_skein2(int thr_id) {}
void free_skunk(int thr_id) {}
void free_sha256t(int thr_id) {}
void free_sha256q(int thr_id) {}
void free_sia(int thr_id) {}
void free_sib(int thr_id) {}
void free_sonoa(int thr_id) {}
void free_s3(int thr_id) {}
void free_vanilla(int thr_id) {}
void free_veltor(int thr_id) {}
void free_whirl(int thr_id) {}
void free_wildkeccak(int thr_id) {}
void free_x11evo(int thr_id) {}
void free_x11(int thr_id) {}
void free_x12(int thr_id) {}
void free_x13(int thr_id) {}
void free_x14(int thr_id) {}
void free_x15(int thr_id) {}
void free_x16r(int thr_id) {}
void free_x16s(int thr_id) {}
void free_x17(int thr_id) {}
void free_zr5(int thr_id) {}
void free_scrypt(int thr_id) {}
void free_scrypt_jane(int thr_id) {}
void free_timetravel(int thr_id) {}
void free_tribus(int thr_id) {}

// Stub functions for equihash and crypto
double equi_network_diff(struct work *work) { return 0.0; }
void equi_work_set_target(struct work* work, double diff) {}
bool equi_stratum_submit(struct pool_infos *pool, struct work *work) { return false; }
bool equi_stratum_notify(struct stratum_ctx *sctx, json_t *params) { return false; }
bool equi_stratum_set_target(struct stratum_ctx *sctx, json_t *params) { return false; }
bool equi_stratum_show_message(struct stratum_ctx *sctx, json_t *id, json_t *params) { return false; }

// Stub for crypto RPC
bool rpc2_stratum_gen_work(struct stratum_ctx *sctx, struct work *work) { return false; }
bool rpc2_stratum_submit(struct pool_infos *pool, struct work *work) { return false; }
void rpc2_stratum_thread_stuff(struct pool_infos *pool) {}
bool rpc2_job_decode(const json_t *job, struct work *work) { return false; }
void rpc2_init() {}
bool rpc2_stratum_authorize(struct stratum_ctx *sctx, const char *user, const char *pass) { return false; }
void rpc2_stratum_job(struct stratum_ctx *sctx, json_t *id, json_t *params) {}

// Stub for sia
bool sia_getheader(void *curl, struct pool_infos *pool) { return false; }
bool sia_work_decode(const char *hexstr, struct work *work) { return false; }
bool sia_submit(void *curl, struct pool_infos *pool, struct work *work) { return false; }

// Stub for scratchpad
char *opt_scratchpad_url = nullptr;
int scratchpad_size = 0;
bool GetScratchpad() { return true; }
