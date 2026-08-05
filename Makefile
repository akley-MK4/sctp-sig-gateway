all: test-run

DOCKER_BUILD_DIR = $(PWD)
CONTAINERD_SOCK = /run/k3s/containerd/containerd.sock

IMAGE_NAME = sctp-sig-gateway
IMAGE_VER = v1.0.0
DOCKERFILE_PATH = $(PWD)/Dockerfile

test-run:
	echo "test"   

build-img:
	sudo docker build -f $(DOCKERFILE_PATH) -t $(IMAGE_NAME):$(IMAGE_VER) $(DOCKER_BUILD_DIR)
	docker save $(IMAGE_NAME):$(IMAGE_VER) > $(IMAGE_NAME)-$(IMAGE_VER).tar
	sudo ctr --address $(CONTAINERD_SOCK) --namespace k8s.io images import $(IMAGE_NAME)-$(IMAGE_VER).tar
	rm $(IMAGE_NAME)-$(IMAGE_VER).tar

local-build-img:
	#bash -c ./build.sh
	sudo docker build -f $(DOCKERFILE_PATH).local -t $(IMAGE_NAME):$(IMAGE_VER) $(DOCKER_BUILD_DIR)
	docker save $(IMAGE_NAME):$(IMAGE_VER) > $(IMAGE_NAME)-$(IMAGE_VER).tar
	sudo ctr --address $(CONTAINERD_SOCK) --namespace k8s.io images import $(IMAGE_NAME)-$(IMAGE_VER).tar
	rm $(IMAGE_NAME)-$(IMAGE_VER).tar

rmi-repo:
	sudo docker rmi $(IMAGE_NAME)

rmi-repo-tag:
	sudo docker rmi $(IMAGE_NAME):$(IMAGE_VER)


.PHONY: build-img local-build-img rmi-repo rmi-repo-tag
