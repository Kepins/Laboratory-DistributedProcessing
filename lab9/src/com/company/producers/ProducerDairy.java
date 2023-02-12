package com.company.producers;

import com.company.products.Butter;
import com.company.products.Cheese;
import com.company.storages.Storage;

import java.util.Random;

public class ProducerDairy implements Runnable{
    private Storage<Cheese> storageCheese;
    private Storage<Butter> storageButter;

    public ProducerDairy(Storage<Cheese> storageCheese, Storage<Butter> storageButter){
        this.storageButter = storageButter;
        this.storageCheese = storageCheese;
    }

    @Override
    public void run() {
        Random rand = new Random();
        while(!Thread.interrupted()){
            if(rand.nextBoolean()) {
                storageCheese.put(new Cheese());
                try{
                    Thread.sleep(25);
                }
                catch (InterruptedException ex){
                    break;
                }
            }
            else{
                storageButter.put(new Butter());
                try{
                    Thread.sleep(25);
                }
                catch (InterruptedException ex){
                    break;
                }
            }
        }
    }
}
