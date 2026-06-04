import sys

import ncempy.io as nio  # 使用该命令来安装 pip install ncempy
import numpy as np


def load_to_ndarray(file_path: str) -> np.ndarray:
    src = nio.mrc.mrcReader(file_path)
    data = src['data'].copy()
    src.clear()
    return data


def scale_image_to_grayscale(data: np.ndarray) -> np.ndarray:
    mi = data.min()
    ma = data.max()
    return ((data - mi) / (ma - mi)) * 255


def save_to_binary(file_path: str, data: np.ndarray):
    fileobj = open(file_path, mode='wb')
    np.asarray(data.shape, dtype=np.dtype('uint32')).tofile(fileobj)
    data.astype('uint8').tofile(fileobj)
    fileobj.close()


# 使用方式：python mrc2binary.py "E:\Temp\cryo_EM\emd_33297.map" "E:\Temp\python-dev\mrcfile\data.dat"
if __name__ == '__main__':
    assert len(sys.argv) == 3  # 参数必须是这个格式：mrc2binary.py mrc文件的路径 输出文件的路径
    input_file_path = sys.argv[1]
    output_file_path = sys.argv[2]
    print(f'Try to load the file from {input_file_path}')
    data = load_to_ndarray(input_file_path)
    print(f'Loaded successfully')
    data = scale_image_to_grayscale(data)
    print(f'Try to save the file to {output_file_path}')
    save_to_binary(output_file_path, data)
    print(f'Saved successfully , data.shape={data.shape}')
    print('Done')

