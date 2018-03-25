"""
Initial import
"""
from __future__ import print_function
from root_numpy import root2array, tree2array
import ROOT



"""
#Get Data
"""
rfile = ROOT.TFile("TreeFile.root")
intree = rfile.Get('Tree')
intree.Print()
# and convert the TTree into an array
array = tree2array(intree,branches=["Gen","Reco","data"])
array.dtype.names = ('reco', "gen", "data")
"""
#Plot Data

"""
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
import numpy as np
import pandas as pd
df = pd.DataFrame(array)

NBins=10
xmin=0
xmax=1000
content_reco ,bin,patches = plt.hist(df['reco'], bins=NBins)
plt.ylabel('# events')
plt.xlabel("reco")
# plt.show()
plt.savefig("plots/reco.pdf")

content_gen ,bin,patches = plt.hist(df['gen'], bins=NBins)
print(content_gen)
plt.ylabel('# events')
plt.xlabel("gen")
plt.savefig("plots/gen.pdf")
# plt.show()

plt.hist2d(df['reco'],df['gen'], bins=NBins, range=[[xmin, xmax], [xmin, xmax]],norm=LogNorm())
plt.ylabel('# events')
plt.colorbar()
plt.savefig("plots/data.pdf")
# plt.show()

"""
#Do unfolding using NN
"""
"""
basic params
"""

batch_size = 500
num_classes = NBins
epochs = 10

"""
#Keras Imports
"""
import keras
from keras.models import Sequential
from keras.layers import Dense
from keras.models import Sequential
from keras.layers import Dense,Flatten

"""
#convert to categorical
"""
gen=df["gen"].values
# print(gen)
# print(gen.shape)
binwidth=(xmax-xmin)/NBins+1
g=np.trunc(gen/binwidth)
# print(g)
reco=df["reco"].values
# print(reco)
# print(reco.shape)
r=np.trunc(reco/binwidth)
print(r)
data=df["data"].values
data=np.trunc(data/binwidth)
data = data.reshape(data.shape[0],1,1)
gcat = keras.utils.to_categorical(g,NBins)
from sklearn.model_selection import train_test_split
r = r.reshape(r.shape[0],1,1)
r_train, r_test, g_train, g_test = train_test_split(r, gcat, test_size=0.2, random_state=42)
"""
#prepare model
"""

def prepareModel(nvar=1, NBins=NBins, kappa=8):
    ''' Prepare KERAS-based sequential neural network with for ML unfolding. 
        Nvar defines number of inputvariables. NBins is number of truth bins. 
        kappa is an empirically tuned parameter for the intermediate layer'''
    model = Sequential()
    model.add(Dense(nvar,activation='linear',input_shape=(nvar,1)))
    model.add(Flatten())
    model.add(Dense(kappa*NBins**2,activation='relu'))
    
    # model.add(Dropout(0.25))
    # model.add(Dense(2*NBins,activation='relu'))
    # model.add(Dropout(0.5))
    model.add(Dense(NBins,activation='softmax'))
    model.compile(loss=keras.losses.categorical_crossentropy,
              optimizer=keras.optimizers.Adadelta(),
              metrics=['accuracy'])
    return model
model = prepareModel(1,NBins)

"""
#fit model
"""

CallBack=keras.callbacks.TensorBoard(log_dir='./logs', histogram_freq=0, batch_size=32, write_graph=True, write_grads=False, write_images=False, embeddings_freq=0, embeddings_layer_names=None, embeddings_metadata=None)
h = model.fit(r_train,g_train,batch_size=batch_size,epochs=epochs,verbose=1,validation_data=(r_test,g_test), callbacks=[CallBack])

"""
save model
"""
model.save("model.hdf5")

"""
evaluate model 
"""
loss_and_metrics = model.evaluate(r_test, g_test, batch_size=128)
print(loss_and_metrics)

"""
test predictions
"""

prob = model.predict(r_test, batch_size=128)
classes=prob.argmax(axis=-1)
np.set_printoptions(threshold=100000)
for i in range(40):    
    print(classes[i],g_test.argmax(axis=-1)[i])

