package com.company.storages;


import java.util.ArrayList;
import java.util.List;

public class Storage<E> implements StorageInterface<E> {
    private List<E> products;
    private int count;
    private final int MAX_COUNT;

    public Storage(int max_count) {
        this.products = new ArrayList<E>();
        this.count = 0;
        this.MAX_COUNT = max_count;
    }

    @Override
    public synchronized E take() throws InterruptedException {
        while(products.isEmpty()){
            wait();
        }
        count--;
        return products.remove(0);
    }

    @Override
    public synchronized void put(E c) {
        if(count<MAX_COUNT) {
            count++;
            products.add(c);
            notifyAll();
        }
    }
}
