import matplotlib.pyplot as plt
import numpy as np
import os
plt.style.use('seaborn')

nSamples = 256
nCols = 33


def load_data(filename, num_events):
    data = np.genfromtxt(filename)
    ped = data[:nSamples].T

    channel_data = [[
            data[nSamples * (i + 1):(i + 2) * nSamples, ch]
         for i in range(num_events)
        ] for ch in range(nCols)]

    return (np.asarray(channel_data), ped)

def plot_event(data,ch, ped, event=0, path=None):
    plt.plot(data[ch + 1, event] - ped[ch+1], label='Evt:{}'.format(event))
    plt.legend()
    plt.title('Channel:Event | {}:{}'.format(ch, event))
    plt.xlabel('Time (100 ps)')
    plt.ylabel('ADC Counts')
    plt.gca().invert_yaxis()
    plt.show()
    if path:
        directory = 'figs/{}'.format(path)
        if not os.path.exists(directory):
            os.makedirs(directory)
        plt.savefig('{}/ch{}-{}'.format(directory, ch, events))
