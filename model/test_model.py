from tensorflow.keras.models import load_model
import joblib
import numpy as np



import numpy as np

def generate_input_sample(num_samples=10, window_size=10, min_temp=22, max_temp=27):
    """
    Hàm tạo tự động các test case với cấu trúc [samples, time steps, features]
    `num_samples`: Số lượng test cases bạn muốn tạo
    `window_size`: Số bước thời gian (tức là số giá trị cần có trong mỗi test case)
    `min_temp` và `max_temp`: Phạm vi giá trị nhiệt độ mà bạn muốn tạo
    """
    # Tạo số lượng test case với giá trị ngẫu nhiên trong phạm vi min_temp và max_temp
    test_cases = []
    for _ in range(num_samples):
        # Mỗi test case có 'window_size' giá trị ngẫu nhiên
        case = np.random.uniform(min_temp, max_temp, window_size)  # Tạo dữ liệu ngẫu nhiên
        case = case.reshape((window_size, 1))  # Reshape để có cấu trúc (time steps, features)
        test_cases.append(case)
    # Chuyển thành mảng numpy 3 chiều (samples, time steps, features)
    return np.array(test_cases)


scaler = joblib.load('scaler.pkl')
# Load mô hình
model = load_model('rnn_model.h5')

max_value = scaler.data_max_[0]
min_value = scaler.data_min_[0]
print("Min:", min_value)
print("Max:", max_value)
# In cấu trúc
model.summary()
# Tạo dữ liệu giả (1 mẫu, 10 bước, 1 đặc trưng)
input_sample = np.array([[[24.10], [26.06], [25.2], [26.20], [26.25], [25.56], [25.7], [23.77], [25], [25.1]]], dtype=np.float32)
print (input_sample.shape)
# Dự đoán
output = model.predict(input_sample)
test_num = 5
val_output = output * (max_value - min_value) + min_value
print("Kết quả dự đoán:", val_output)


test_cases = generate_input_sample(num_samples=test_num, window_size=10)
# print(test_cases.shape)  
# print(test_cases[0].reshape(1,10,1))  # In test case đầu tiên
for i in range(test_num):
    print("Test case", i+1, ":", test_cases[i].reshape(-1))
    print()
    print("Dự đoán:", model.predict(test_cases[i].reshape(1,10,1)))
    print("Dự đoán sau khi scale:", model.predict(test_cases[i].reshape(1,10,1)) * (max_value - min_value) + min_value)
    print("----------------------------------------------------------------------------------------")