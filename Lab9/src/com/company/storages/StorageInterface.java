package com.company.storages;

public interface StorageInterface<E> {
    E take() throws InterruptedException;
    void put(E e);
}
