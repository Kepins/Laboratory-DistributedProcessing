package com.company.producers;


import com.company.products.Ham;
import com.company.storages.Storage;

public class ProducerHam implements Runnable {
    private Storage<Ham> storage;

    public ProducerHam(Storage<Ham> storage){
        this.storage = storage;
    }

    @Override
    public void run() {
        while(!Thread.interrupted()){
            storage.put(new Ham());
            try{
                Thread.sleep(30);
            }
            catch (InterruptedException ex){
                break;
            }
        }
    }
}
