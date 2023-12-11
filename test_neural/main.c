#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "util/img.h"
#include "neural/activations.h"
#include "neural/nn.h"
#include "matrix/matrix.h"
#include "matrix/ops.h"

int main(int argc, char** argv) {
	srand(time(NULL));

	int number_imgs = 0;
	Img** imgs;
	NeuralNetwork* net;

	if (argc > 1){
	//TRAINING
	number_imgs = 1700;
	 imgs = csv_to_imgs("mnist_train.csv", number_imgs);
	 net = network_create(784, 300, 10, 0.1);
	 network_train_batch_imgs(net, imgs, number_imgs);
	 network_save(net, "testing_net");
	}
	else{
	// PREDICTING
	 number_imgs = 2000;
	 imgs = csv_to_imgs("mnist_test.csv", number_imgs);
	 net = network_load("testing_net");
	 double score = network_predict_imgs(net, imgs, number_imgs);
	printf("Score: %1.5f\n", score * 100);
	}
	 imgs_free(imgs, number_imgs);
	 network_free(net);
	return 0;
}