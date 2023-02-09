package com.company.producers;

import com.company.products.Bread;

import com.company.storages.Storage;

public class ProducerBread implements Runnable {
    private Storage<Bread> storage;

    public ProducerBread(Storage<Bread> storage){
        this.storage = storage;
    }



    @Override
    public void run() {
        while(!Thread.interrupted()){
            storage.put(new Bread());
            try{
                Thread.sleep(30);
            }
            catch (InterruptedException ex){
                break;
            }
        }
    }
}