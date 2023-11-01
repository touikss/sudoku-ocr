#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define LEARNING_RATE 0.1


// we choose sigmoid function because this is the easyiest function to work with. 
float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

float sigmoid_derivative(float x) {
    return x * (1.0f - x);
}

typedef struct Neuron {
    float weights[2];
    float bias;
} Neuron;


void init_neuron(Neuron *neuron) {
    for (int i = 0; i < 2; i++) {
        neuron->weights[i] = (float)rand() / RAND_MAX;
    }
    neuron->bias = (float)rand() / RAND_MAX;
}

//it return the value of the neuron which is the sum of all weights + the bias into the sigmoid function
float neuronOutput(Neuron *neuron, float inputs[2]) {
    float sum = 0.0f;
    for (int i = 0; i < 2; i++) {
        sum += neuron->weights[i] * inputs[i];
    }
    sum += neuron->bias;
    return sigmoid(sum);
}


void adjustWeights(Neuron *neuron, float error, float inputs[2]) {
  // thanks to the error we can adjust each weights and the bias 
    for (int i = 0; i < 2; i++) {
        neuron->weights[i] += LEARNING_RATE * error * sigmoid_derivative(neuronOutput(neuron, inputs)) * inputs[i];
    }
    neuron->bias += LEARNING_RATE * error * sigmoid_derivative(neuronOutput(neuron, inputs));
}

int main() {
    int nb_tests = 100000;
    int display_interval = 10000;


    Neuron hiddenLayer[5];
    for (int i = 0; i < 5; i++) {
        init_neuron(&hiddenLayer[i]);
    }

    // tabs which contains values of inputs and expected outputs
    float input[4][2] = {{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}};
    float expectedOutput[4] = {0.0f, 1.0f, 1.0f, 0.0f};

    // here we are testing neurons 
    for (int t = 0; t < nb_tests; t++) { // for each tests 
        for (int i = 0; i < 4; i++) {
            float hiddenOutputs[5];
            for (int j = 0; j < 5; j++) {
                hiddenOutputs[j] = neuronOutput(&hiddenLayer[j], input[i]);
            }

            // here we calculate the final output
            float output = 0.0f;
            for (int j = 0; j < 5; j++) {
                output += hiddenOutputs[j];
            }
            
            // error is the difference between the real output and our actual output
            float error = expectedOutput[i] - output;
            
            // we can adjust the weights thanks to the error
            for (int j = 0; j < 5; j++) {
                adjustWeights(&hiddenLayer[j], error, input[i]);
            }
        }

        // we could also have tested when "t % display_interval == 0" but it looked less beautiful if the numbers were not round
        if ((t + 1) % display_interval == 0) {
            printf("Iteration %d:\n", t + 1);
            for (int i = 0; i < 4; i++) {
                float hiddenOutputs[5];
                for (int j = 0; j < 5; j++) {
                    hiddenOutputs[j] = neuronOutput(&hiddenLayer[j], input[i]);
                }

                float output = 0.0f;
                for (int j = 0; j < 5; j++) {
                    output += hiddenOutputs[j];
                }

                float actualOutput = expectedOutput[i];
                printf("Input: %i %i, Predicted Output: %.2f, Expected Output: %i\n", (int)input[i][0], (int)input[i][1], output,(int)actualOutput);
            }
        }
    }

    return 0;
}

