import tensorflow as tf

# Tải mô hình đã huấn luyện
model = tf.keras.models.load_model('rnn_model.h5')

# Chuyển đổi mô hình thành TensorFlow Lite
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

# Lưu mô hình TFLite
with open('model.tflite', 'wb') as f:
    f.write(tflite_model)

# Dung lenh xxd -i model.tflite > model_data.cc trong gitbash