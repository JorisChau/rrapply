FROM rocker/r-devel

ARG PKG_VER=1.2.8

WORKDIR /home/

COPY rrapply_${PKG_VER}.tar.gz .
RUN Rdevel CMD INSTALL rrapply_${PKG_VER}.tar.gz

CMD ["Rdevel"]