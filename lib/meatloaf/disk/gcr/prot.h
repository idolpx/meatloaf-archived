/* prot.h */
void shift_buffer(uint8_t * buffer, int length, int n);
uint8_t *align_vmax(uint8_t * work_buffer, int track_len);
uint8_t *auto_gap(uint8_t * work_buffer, int track_len);
uint8_t *find_weak_gap(uint8_t * work_buffer, int tracklen);
uint8_t *find_long_sync(uint8_t * work_buffer, int tracklen);
uint8_t *auto_gap(uint8_t * work_buffer, int tracklen);

