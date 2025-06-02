#ifndef MODEL_DATA_H_
#define MODEL_DATA_H_

extern const unsigned char model_tflite[];
extern const unsigned int model_tflite_len;

// Có thể define alias tên ngắn hơn cho dễ dùng:
#define g_model_data model_tflite
#define g_model_data_len model_tflite_len

#endif  // MODEL_DATA_H_
