package com.company.consumers;

import com.company.products.Bread;
import com.company.products.Butter;
import com.company.products.Cheese;
import com.company.storages.Storage;

public class ConsumerSandwichCheese implements Runnable{
    private Storage<Bread> breads;
    private Storage<Cheese> cheeses;
    private Storage<Butter> butter;

    public ConsumerSandwichCheese(Storage<Bread> breads, Storage<Cheese> cheeses ,Storage<Butter> butter){
        this.breads = breads;
        this.cheeses = cheeses;
        this.butter = butter;
    }


    @Override
    public void run() {
        while (!Thread.interrupted()) {
            try {
                breads.take();
                cheeses.take();
                butter.take();
                System.out.println("Consumer made sandwich with cheese");
            } catch (InterruptedException ex) {
                break;
            }
        }
    }
}