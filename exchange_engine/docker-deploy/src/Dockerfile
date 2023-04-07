From ubuntu:20.04
RUN apt-get update && \
    apt-get install -yq tzdata && \
    ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime && \
    dpkg-reconfigure -f noninteractive tzdata
ENV TZ="America/New_York"

RUN apt-get update
RUN apt-get install -y g++ make libboost-all-dev postgresql libpq-dev
RUN apt-get install -y libpqxx-dev build-essential libpugixml-dev libssl-dev
RUN mkdir /code
WORKDIR /code
ADD . /code/

RUN chmod u+x launch.sh

