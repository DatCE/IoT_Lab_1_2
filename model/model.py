import numpy as np
import joblib
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import GRU, Dense, Input, SimpleRNN
from tensorflow.keras.losses import MeanSquaredError
from sklearn.preprocessing import MinMaxScaler
# Tạo dữ liệu giả (nhiệt độ có nhiễu)
data = 25 + np.random.normal(0, 0.5, 3000)
# data[100] = 100  # nhiễu
# data[300] = 80   # nhiễu
# print (data)
# Scale dữ liệu
# print(np.min(data), np.max(data))
# scaler = MinMaxScaler()
# data_scaled = scaler.fit_transform(data.reshape(-1, 1))
# # print(data_scaled)
# joblib.dump(scaler, 'scaler.pkl')

# print("Min:", scaler.data_min_[0])
# print("Max:", scaler.data_max_[0])

# Tạo tập train (10 giá trị đầu -> dự đoán giá trị thứ 11)
def create_sequence(data, window=10):
    X, y = [], []
    for i in range(len(data) - window):
        X.append(data[i:i+window])
        y.append(data[i+window])
    return np.array(X), np.array(y)

window_size = 10
X, y = create_sequence(data, window_size)
print(X.shape)
X = X.reshape((X.shape[0], X.shape[1], 1))  # (samples, time steps, features)

# Mô hình GRU thay vì LSTM
model = Sequential([
    Input(shape=(window_size, 1)),
    SimpleRNN(16, activation='tanh', unroll=True),  # Dễ convert sang TFLite
    Dense(8, activation='relu'),
    Dense(1)
])
model.compile(optimizer='adam', loss=MeanSquaredError())
model.fit(X, y, epochs=20, batch_size=32)

# Lưu model
model.save("rnn_model.h5")