import numpy as np
def is_float_try(str):
    try:
        float(str)
        return True
    except ValueError:
        return False
with open('out_running.txt', 'r') as file:
    st = file.read()
    st = st.replace('<Matrix 4x4','')
    st = st.replace('>','')
    st = st.replace('(','')
    st = st.replace(')','')
    st = st.replace(',',' ')
    st = st.replace('Frame','')
print(st)
file1 = open("running_mat.txt","w")
file1.write(st)
file1.close() 
